// Pure selection logic. Never guess a different chip family or a smaller flash.
export function selectFirmware(chipName, detectedSize, profiles) {
  const name = String(chipName || '').toUpperCase();
  const family = ['ESP32', 'ESP32-S2', 'ESP32-S3'].includes(name) ? name : null;
  const size = /^([0-9]+)MB$/i.exec(String(detectedSize || ''));
  if (!family || !size) return null;
  const megabytes = Number(size[1]);
  const capacity = family === 'ESP32-S3' ? megabytes : 4;
  if (megabytes < 4 || (family === 'ESP32-S3' && ![4, 8, 16].includes(megabytes))) return null;
  const profile = Object.values(profiles || {}).find(item =>
    item.chipFamily === family && item.flashMB === capacity);
  return profile || null;
}
