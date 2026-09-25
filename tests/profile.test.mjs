import test from 'node:test';
import assert from 'node:assert/strict';
import {selectFirmware} from '../flasher/profile.mjs';

const profiles = {
  's3-4': {chipFamily:'ESP32-S3',flashMB:4},
  's3-8': {chipFamily:'ESP32-S3',flashMB:8},
  's3-16': {chipFamily:'ESP32-S3',flashMB:16},
  'esp32-4': {chipFamily:'ESP32',flashMB:4},
  's2-4': {chipFamily:'ESP32-S2',flashMB:4}
};

test('detecta la familia y selecciona capacidad de S3 sin intervención', () => {
  for (const capacity of [4,8,16]) assert.equal(selectFirmware('ESP32-S3', `${capacity}MB`, profiles), profiles[`s3-${capacity}`]);
});
test('usa partición común de 4 MB para ESP32 clásico y S2 con flash mayor', () => {
  assert.equal(selectFirmware('ESP32','8MB',profiles), profiles['esp32-4']);
  assert.equal(selectFirmware('ESP32-S2','16MB',profiles), profiles['s2-4']);
});
test('se detiene en familias y memorias no publicadas', () => {
  for (const input of [['ESP32-C3','4MB'],['ESP32-S3','2MB'],['ESP32-S3','32MB'],['ESP32-S3',undefined],['ESP32-C6','8MB']])
    assert.equal(selectFirmware(...input,profiles),null);
});
