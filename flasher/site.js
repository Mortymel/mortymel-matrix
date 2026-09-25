(() => {
  const support = document.getElementById('support');
  if (support && (!window.isSecureContext || !('serial' in navigator))) {
    support.textContent = 'Este navegador no ofrece Web Serial. Usa Chrome o Edge de escritorio en HTTPS.';
    support.classList.add('callout');
  }
  const selector = document.getElementById('flash-profile');
  const meta = document.getElementById('build-meta');
  let build;
  function showProfile() {
    const capacity = selector.value;
    document.querySelectorAll('.profile-installer').forEach(el => { el.hidden = el.dataset.profile !== capacity; });
    if (!build) return;
    const data = build.profiles[capacity];
    document.getElementById('usb-download').href = './' + data.usb.path;
    document.getElementById('ota-download').href = './' + data.ota.path;
    meta.replaceChildren();
    const label = document.createElement('strong');
    label.textContent = `Versión ${build.version} · S3 QD ${capacity} MB · Imagen USB: ${data.usb.bytes.toLocaleString('es')} bytes · OTA: ${data.ota.bytes.toLocaleString('es')} bytes`;
    meta.append(label, document.createElement('br'));
    for (const [kind, item] of [['USB', data.usb], ['OTA', data.ota]]) {
      const line = document.createElement('span');
      line.textContent = `${kind} SHA-256: `;
      const hash = document.createElement('code');
      hash.textContent = item.sha256;
      line.append(hash);
      meta.append(line, document.createElement('br'));
    }
  }
  selector.addEventListener('change', showProfile);
  showProfile();
  fetch('./build.json', {cache: 'no-store'}).then(response => {
    if (!response.ok) throw new Error('metadata');
    return response.json();
  }).then(data => { build = data; showProfile(); }).catch(() => {
    meta.textContent = 'No se pudieron cargar las huellas; consulta build.json antes de descargar.';
  });
})();
