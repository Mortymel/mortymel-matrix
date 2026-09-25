(() => {
  const support = document.getElementById('support');
  if (support && (!window.isSecureContext || !('serial' in navigator))) {
    support.textContent = 'Este navegador no ofrece Web Serial. Usa Chrome o Edge de escritorio en HTTPS.';
    support.classList.add('callout');
  }
  const meta = document.getElementById('build-meta');
  if (!meta) return;
  fetch('./build.json', {cache: 'no-store'}).then(response => {
    if (!response.ok) throw new Error('metadata');
    return response.json();
  }).then(data => {
    meta.replaceChildren();
    const label = document.createElement('strong');
    label.textContent = `Versión ${data.version} · Imagen USB: ${data.usb.bytes.toLocaleString('es')} bytes · OTA: ${data.ota.bytes.toLocaleString('es')} bytes`;
    meta.append(label, document.createElement('br'));
    for (const [kind, item] of [['USB', data.usb], ['OTA', data.ota]]) {
      const line = document.createElement('span');
      line.textContent = `${kind} SHA-256: `;
      const hash = document.createElement('code');
      hash.textContent = item.sha256;
      line.append(hash);
      meta.append(line, document.createElement('br'));
    }
  }).catch(() => { meta.textContent = 'Las huellas se publican junto a cada compilación; consulta el archivo build.json del sitio.'; });
})();
