import {selectFirmware} from './profile.mjs';

const button = document.getElementById('auto-install');
const state = document.getElementById('install-state');
const progress = document.getElementById('install-progress');
const support = document.getElementById('support');
const downloads = document.getElementById('descargas');
const meta = document.getElementById('build-meta');
const log = document.getElementById('serial-log');
const monitorButton = document.getElementById('monitor-toggle');
let build, monitorPort, monitorReader, monitorActive = false;
const status = message => { state.textContent = message; };

if (!window.isSecureContext || !('serial' in navigator)) {
  button.disabled = true;
  support.textContent = 'Necesitas un navegador con Web Serial en HTTPS, como Chrome o Edge de escritorio.';
  support.classList.add('callout');
}

function showFiles(profile) {
  document.getElementById('usb-download').href = './' + profile.usb.path;
  document.getElementById('ota-download').href = './' + profile.ota.path;
  downloads.hidden = false;
  meta.replaceChildren();
  const label = document.createElement('strong');
  label.textContent = `Versión ${build.version} · ${profile.chipFamily} · ${profile.flashMB} MB`;
  meta.append(label, document.createElement('br'));
  for (const [kind, item] of [['USB', profile.usb], ['OTA', profile.ota]]) {
    const line = document.createElement('span');
    line.textContent = `${kind} · ${item.bytes.toLocaleString('es')} bytes · SHA-256: `;
    const hash = document.createElement('code');
    hash.textContent = item.sha256;
    line.append(hash);
    meta.append(line, document.createElement('br'));
  }
}

async function verifiedImage(item, bootOffset) {
  const response = await fetch('./' + item.path, {cache: 'no-store'});
  if (!response.ok) throw new Error('No se pudo descargar el firmware.');
  const data = new Uint8Array(await response.arrayBuffer());
  const hash = [...new Uint8Array(await crypto.subtle.digest('SHA-256', data))]
    .map(byte => byte.toString(16).padStart(2, '0')).join('');
  if (data.length !== item.bytes || hash !== item.sha256) {
    throw new Error('El firmware descargado no coincide con su huella SHA-256.');
  }
  if (data[0x10000] !== 0xe9 || data[bootOffset] !== 0xe9) {
    throw new Error('La imagen no tiene las cabeceras ESP esperadas.');
  }
  return data;
}

button.addEventListener('click', async () => {
  if (monitorActive) return status('Cierra primero el registro serie.');
  button.disabled = true;
  let transport;
  try {
    if (!build) throw new Error('No se pudo cargar el catálogo de firmware. Recarga la página.');
    const {ESPLoader, Transport} = await import('https://unpkg.com/esptool-js@0.7.0/bundle.js');
    const port = await navigator.serial.requestPort();
    transport = new Transport(port, true);
    const loader = new ESPLoader({transport, baudrate: 115200, terminal: {
      clean() {}, writeLine() {}, write() {}
    }});
    status('Detectando chip y flash…');
    const chipName = await loader.main();
    const flashSize = await loader.detectFlashSize();
    status(`Detectado: ${chipName} · flash ${flashSize || 'no identificada'}.`);
    const profile = selectFirmware(loader.chip.CHIP_NAME, flashSize, build.profiles);
    if (!profile) throw new Error(`No hay una imagen verificada para ${chipName} con flash ${flashSize || 'desconocida'}. No se ha escrito nada.`);
    showFiles(profile);
    const image = await verifiedImage(profile.usb, profile.bootOffset);
    if (!confirm(`Detectado ${chipName}, flash ${flashSize}. Se instalará Mortymel Matrix. Una instalación nueva borrará TODO lo que haya en la flash: firmware, ajustes y GIF. ¿Continuar?`)) {
      status('Instalación cancelada. No se ha escrito nada.');
      return;
    }
    progress.hidden = false;
    progress.value = 0;
    status('Instalando firmware… Mantén conectado el cable USB.');
    await loader.writeFlash({
      fileArray: [{data: image, address: 0}], flashMode: 'keep', flashFreq: 'keep',
      flashSize: 'keep', eraseAll: true, compress: true,
      reportProgress(_index, written, total) { progress.value = Math.round(100 * written / total); }
    });
    status('Firmware instalado. Abre Registro serie, selecciona el puerto y pulsa RESET en la placa.');
    await loader.after('hard_reset');
  } catch (error) {
    status(error.name === 'NotFoundError' ? 'No se eligió ningún puerto.' : 'Error: ' + error.message);
  } finally {
    if (transport) try { await transport.disconnect(); } catch (_) { /* Port may reset automatically. */ }
    button.disabled = false;
  }
});

monitorButton.addEventListener('click', async () => {
  if (monitorActive) {
    monitorActive = false;
    if (monitorReader) await monitorReader.cancel().catch(() => {});
    return;
  }
  if (button.disabled) return;
  try {
    monitorPort = await navigator.serial.requestPort();
    await monitorPort.open({baudRate: 115200});
    monitorActive = true;
    monitorButton.textContent = 'Cerrar registro';
    button.disabled = true;
    log.textContent = 'Puerto abierto. Pulsa RESET en el ESP32 para leer las credenciales.\n';
    const decoder = new TextDecoder();
    while (monitorActive && monitorPort.readable) {
      monitorReader = monitorPort.readable.getReader();
      try {
        while (monitorActive) {
          const {value, done} = await monitorReader.read();
          if (done) break;
          log.textContent = (log.textContent + decoder.decode(value, {stream: true})).slice(-12000);
          log.scrollTop = log.scrollHeight;
        }
      } finally { monitorReader.releaseLock(); monitorReader = null; }
      break;
    }
  } catch (error) { log.textContent += '\nError: ' + error.message; }
  finally {
    monitorActive = false;
    if (monitorPort) try { await monitorPort.close(); } catch (_) {}
    monitorPort = null;
    monitorButton.textContent = 'Abrir registro a 115200';
    button.disabled = false;
  }
});

fetch('./build.json', {cache: 'no-store'}).then(response => {
  if (!response.ok) throw new Error('Catálogo no disponible');
  return response.json();
}).then(data => { build = data; }).catch(error => status('Error: ' + error.message));
