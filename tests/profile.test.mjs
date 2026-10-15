import test from 'node:test';
import assert from 'node:assert/strict';
import {selectFirmware, compatibleChip} from '../flasher/profile.mjs';
const profiles = {
  's3-4': {chipFamily:'ESP32-S3',flashMB:4},
  's3-8': {chipFamily:'ESP32-S3',flashMB:8},
  's3-16': {chipFamily:'ESP32-S3',flashMB:16},
  'esp32-4': {chipFamily:'ESP32',flashMB:4},
  's2-4': {chipFamily:'ESP32-S2',flashMB:4}
};
test('conserva exactamente cada perfil elegido manualmente', () => {
  for (const [id, profile] of Object.entries(profiles)) assert.equal(selectFirmware(id, profiles), profile);
});
test('no selecciona un perfil por defecto ni acepta claves heredadas', () => {
  for (const id of ['', undefined, 'ESP32-S3', 'unknown', 'toString', '__proto__']) assert.equal(selectFirmware(id, profiles), null);
});
test('la comprobación USB rechaza otra familia sin cambiar el perfil', () => {
  for (const profile of Object.values(profiles)) {
    assert.equal(compatibleChip(profile, profile.chipFamily), true);
    assert.equal(compatibleChip(profile, 'ESP32-C3'), false);
  }
  assert.equal(compatibleChip(profiles['s3-8'], 'ESP32'), false);
  assert.equal(compatibleChip(null, 'ESP32'), false);
});
