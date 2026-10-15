(() => {
  const key = 'mortys-theme';
  const preference = window.matchMedia('(prefers-color-scheme: dark)');
  let saved;
  try { saved = localStorage.getItem(key); } catch (_) { /* Storage can be unavailable. */ }
  let explicit = saved === 'dark' || saved === 'light';
  let current = explicit ? saved : (preference.matches ? 'dark' : 'light');
  function apply(theme) {
    current = theme;
    document.documentElement.dataset.theme = theme;
    const dark = theme === 'dark';
    const meta = document.querySelector('meta[name="theme-color"]');
    if (meta) meta.content = dark ? '#151c16' : '#f4f3ec';
    const toggle = document.getElementById('theme-toggle');
    if (toggle) {
      const label = dark ? 'Activar tema claro' : 'Activar tema oscuro';
      toggle.setAttribute('aria-label', label);
      toggle.title = label;
      toggle.querySelector('[data-theme-icon]').textContent = dark ? '☀' : '◐';
      toggle.querySelector('[data-theme-label]').textContent = dark ? 'Claro' : 'Oscuro';
    }
  }
  apply(current);
  function bindToggle() {
    apply(current);
    document.getElementById('theme-toggle')?.addEventListener('click', () => {
      explicit = true;
      apply(current === 'dark' ? 'light' : 'dark');
      try { localStorage.setItem(key, current); } catch (_) { /* Theme still works without storage. */ }
    });
  }
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', bindToggle, {once: true});
  } else {
    bindToggle();
  }
  preference.addEventListener('change', event => {
    if (!explicit) apply(event.matches ? 'dark' : 'light');
  });
})();
