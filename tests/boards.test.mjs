import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {selectFirmware} from '../flasher/profile.mjs';
const boards = JSON.parse(readFileSync(new URL('../flasher/boards.json', import.meta.url)));
const profiles = Object.fromEntries(['esp32', 's2', 's3'].flatMap(family => [4,8,16].map(size => [`${family}-${size}`, {family, size}])));
test('todos los módulos apuntan a un perfil publicado sin opciones duplicadas', () => {
  const labels = new Set();
  for (const board of boards) {
    assert.ok(board.group && board.label);
    assert.ok(selectFirmware(board.profile, profiles), board.label);
    assert.equal(labels.has(board.label), false, board.label);
    labels.add(board.label);
  }
});
test('cada familia y capacidad se puede elegir y no se anuncian chips incompatibles', () => {
  assert.deepEqual(new Set(boards.map(board => board.profile)), new Set(Object.keys(profiles)));
  assert.equal(boards.some(board => /ESP32-(C\d|H\d|P\d)|WROOM-2/.test(board.label)), false);
});
