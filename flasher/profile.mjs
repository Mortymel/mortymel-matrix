// The user selects the published profile; USB identification never selects firmware.
export function selectFirmware(profileId, profiles) {
  return Object.hasOwn(profiles || {}, profileId) ? profiles[profileId] : null;
}
export function compatibleChip(profile, chipName) {
  return Boolean(profile && profile.chipFamily === chipName);
}
