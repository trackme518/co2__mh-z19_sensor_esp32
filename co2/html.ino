const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8">
  <title>MH-Z19 CO2 Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    :root {
      --bg-1: #0f1419;
      --bg-2: #1a2430;
      --card: #141c24;
      --card-border: #2a3a4a;
      --text: #e7edf3;
      --muted: #9fb0c0;
      --accent: #40d9a2;
      --danger: #ff6b6b;
      --shadow: rgba(0, 0, 0, 0.35);
    }
    * {
      box-sizing: border-box;
    }
    body {
      margin: 0;
      font-family: "Space Grotesk", "Manrope", "Segoe UI", system-ui, sans-serif;
      color: var(--text);
      background: radial-gradient(1200px 700px at 10% -10%, #1e2c3a 0%, transparent 60%),
        radial-gradient(1200px 700px at 90% 10%, #132b2c 0%, transparent 55%),
        linear-gradient(160deg, var(--bg-1), var(--bg-2));
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }
    .app {
      width: 100%;
      max-width: 900px;
      display: grid;
      gap: 16px;
    }
    header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
    }
    h1 {
      font-size: 1.6rem;
      margin: 0;
      letter-spacing: 0.5px;
    }
    .badge {
      font-size: 0.85rem;
      color: var(--muted);
      padding: 6px 10px;
      border: 1px solid var(--card-border);
      border-radius: 999px;
      background: rgba(255, 255, 255, 0.04);
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 12px;
    }
    .card {
      background: linear-gradient(160deg, rgba(255, 255, 255, 0.03), rgba(255, 255, 255, 0.015));
      border: 1px solid var(--card-border);
      border-radius: 16px;
      padding: 16px;
      box-shadow: 0 12px 30px var(--shadow);
    }
    .metric {
      display: grid;
      gap: 8px;
    }
    .metric-label {
      color: var(--muted);
      font-size: 0.9rem;
    }
    .metric-value {
      font-size: 2.2rem;
      font-weight: 600;
      letter-spacing: 0.5px;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    .metric-value small {
      font-size: 1rem;
      color: var(--muted);
      margin-left: 6px;
    }
    .mood-icon {
      width: 40px;
      height: 40px;
      display: inline-flex;
      align-items: center;
      justify-content: center;
    }
    .mood-icon svg {
      width: 100%;
      height: 100%;
      display: block;
    }
    .controls {
      display: grid;
      gap: 12px;
    }
    .info-grid {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 12px;
    }
    .info-item {
      display: grid;
      gap: 4px;
    }
    .info-item span {
      color: var(--muted);
      font-size: 0.85rem;
    }
    .info-item strong {
      display: block;
    }
    .inline-field {
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      gap: 8px;
    }
    .row {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      align-items: center;
    }
    .row.toggle-group {
      display: grid;
      grid-template-columns: 1fr 1fr;
      align-items: center;
      gap: 10px;
    }
    .row.toggle-group .inline-field {
      width: 100%;
      justify-content: flex-start;
    }
    button {
      background: transparent;
      color: var(--text);
      border: 1px solid var(--card-border);
      padding: 10px 14px;
      border-radius: 10px;
      font-weight: 600;
      cursor: pointer;
      transition: transform 0.08s ease, filter 0.2s ease, opacity 0.2s ease;
      min-height: 40px;
    }
    button:hover {
      filter: brightness(1.05);
    }
    button:active {
      transform: translateY(1px);
    }
    button.secondary {
      background: transparent;
      color: var(--text);
      border: 1px solid var(--card-border);
    }
    button.danger {
      background: var(--danger);
      color: #200b0b;
    }
    button:disabled {
      opacity: 0.55;
      cursor: not-allowed;
    }
    .toggle-group button.active {
      background: var(--accent);
      color: #0b1318;
      border-color: transparent;
      box-shadow: 0 0 0 2px rgba(64, 217, 162, 0.35);
      filter: brightness(1.02);
    }
    .toggle-group button:not(.active).danger {
      opacity: 0.85;
    }
    label {
      color: var(--muted);
      font-size: 0.9rem;
    }
    input[type="number"] {
      background: #0f161d;
      color: var(--text);
      border: 1px solid var(--card-border);
      border-radius: 10px;
      padding: 10px 12px;
      min-width: 120px;
      min-height: 40px;
    }
    #status {
      color: var(--muted);
      font-size: 0.9rem;
      min-height: 1.4em;
      height: 1.4em;
      line-height: 1.4em;
      white-space: nowrap;
      overflow: hidden;
      text-overflow: ellipsis;
    }
    @media (max-width: 700px) {
      body {
        padding: 14px;
      }
      h1 {
        font-size: 1.35rem;
      }
      .grid {
        grid-template-columns: 1fr;
      }
      .metric-value {
        font-size: 2rem;
      }
      .row {
        flex-direction: column;
        align-items: stretch;
      }
      .info-grid {
        grid-template-columns: 1fr;
      }
      input[type="number"] {
        width: 100%;
      }
      button {
        width: 100%;
      }
    }
  </style>
</head>
<body>
  <div class="app">
    <header>
      <h1>MH-Z19 CO2 Monitor</h1>
      <div class="badge">Live Sensor</div>
    </header>
    <div class="grid">
      <div class="card metric">
        <div class="metric-label">CO2</div>
        <div class="metric-value"><span id="co2">--</span><small>ppm</small> <span id="mood" class="mood-icon"></span></div>
      </div>
      <div class="card metric">
        <div class="metric-label">Temperature</div>
        <div class="metric-value"><span id="temp">--</span><small>C</small></div>
      </div>
    </div>
    <div class="card controls">
      <div class="row">
        <div class="metric-label">ABC</div>
        <div class="metric-value" id="abc">--</div>
      </div>
      <div class="row toggle-group">
        <div class="inline-field">
          <button id="abcOnBtn" type="button" onclick="setABC(true)">ABC On</button>
          <button id="abcOffBtn" type="button" class="secondary" onclick="setABC(false)">ABC Off</button>
          <button type="button" class="secondary" onclick="refreshABC()">Refresh ABC</button>
        </div>
        <div class="inline-field">
          <button type="button" class="danger" onclick="calibrate()">Calibrate (Use Current Air)</button>
          <button type="button" class="secondary" onclick="resetDevice()">Reset MCU</button>
        </div>
      </div>
      <div class="info-grid">
        <div class="info-item">
          <span>Threshold (ppm)</span>
          <div class="inline-field">
            <input id="threshold" type="number" min="400" max="5000" step="50" placeholder="1000">
            <button type="button" onclick="saveThreshold()">Save Threshold</button>
          </div>
        </div>
        <div class="info-item">
          <span>Range</span>
          <div class="inline-field">
            <input id="range" type="number" min="1000" max="10000" step="100" placeholder="2000">
            <button type="button" class="secondary" onclick="saveRange()">Save</button>
          </div>
        </div>
        <div class="info-item"><span>Firmware</span><strong id="fw">--</strong></div>
        <div class="info-item"><span>Background CO2</span><strong id="bgco2">--</strong></div>
      </div>
      <div id="status"></div>
    </div>
  </div>
  <script>
    const thresholdInput = document.getElementById('threshold');
    const rangeInput = document.getElementById('range');

    function setStatus(msg) {
      const el = document.getElementById('status');
      el.textContent = msg;
      if (msg) {
        setTimeout(() => { el.textContent = ''; }, 2500);
      }
    }

    function syncThreshold(value) {
      if (document.activeElement !== thresholdInput) {
        thresholdInput.value = value;
      }
    }

    function syncRange(value) {
      if (document.activeElement !== rangeInput) {
        rangeInput.value = value;
      }
    }

    function updateABCButtons(isOn) {
      const onBtn = document.getElementById('abcOnBtn');
      const offBtn = document.getElementById('abcOffBtn');
      if (!onBtn || !offBtn) return;
      if (isOn) {
        onBtn.classList.add('active');
        offBtn.classList.remove('active');
      } else {
        onBtn.classList.remove('active');
        offBtn.classList.add('active');
      }
    }

    function renderMood(state) {
      if (state === 'sad') {
        return `
<svg viewBox="0 0 120 120" aria-label="Sad">
  <defs>
    <radialGradient id="sadFace" cx="35%" cy="35%" r="70%">
      <stop offset="0%" stop-color="#ffd36a"/>
      <stop offset="60%" stop-color="#ffb347"/>
      <stop offset="100%" stop-color="#ff8a3d"/>
    </radialGradient>
    <linearGradient id="tear" x1="0" x2="0" y1="0" y2="1">
      <stop offset="0%" stop-color="#6fd3ff"/>
      <stop offset="100%" stop-color="#2f8cff"/>
    </linearGradient>
  </defs>
  <circle cx="60" cy="60" r="52" fill="url(#sadFace)" stroke="#f07a2d" stroke-width="4"/>
  <circle cx="42" cy="48" r="7" fill="#2f2f2f"/>
  <circle cx="78" cy="48" r="7" fill="#2f2f2f"/>
  <path d="M38 78c8-10 36-10 44 0" fill="none" stroke="#2f2f2f" stroke-width="6" stroke-linecap="round"/>
  <path d="M78 62c6 10 6 18 0 26c-6-8-6-16 0-26z" fill="url(#tear)"/>
</svg>`;
      }
      return `
<svg viewBox="0 0 120 120" aria-label="Happy">
  <defs>
    <radialGradient id="happyFace" cx="35%" cy="35%" r="70%">
      <stop offset="0%" stop-color="#fff199"/>
      <stop offset="60%" stop-color="#ffd166"/>
      <stop offset="100%" stop-color="#ffb347"/>
    </radialGradient>
  </defs>
  <circle cx="60" cy="60" r="52" fill="url(#happyFace)" stroke="#f2a93b" stroke-width="4"/>
  <circle cx="42" cy="48" r="7" fill="#2f2f2f"/>
  <circle cx="78" cy="48" r="7" fill="#2f2f2f"/>
  <path d="M36 70c10 16 38 16 48 0" fill="none" stroke="#2f2f2f" stroke-width="6" stroke-linecap="round"/>
</svg>`;
    }

    async function updateData() {
      try {
        const response = await fetch('/data');
        if (!response.ok) return;
        const data = await response.json();
        document.getElementById('co2').textContent = data.co2;
        document.getElementById('temp').textContent = data.temp;
        document.getElementById('mood').innerHTML = renderMood(data.mood);
        document.getElementById('abc').textContent = data.abc ? 'ON' : 'OFF';
        updateABCButtons(!!data.abc);
        syncThreshold(data.threshold);
        if (data.range !== undefined) syncRange(data.range);
        const calButton = document.querySelector('button[onclick="calibrate()"]');
        if (data.calRemainingMs && data.calRemainingMs > 0) {
          const seconds = Math.ceil(data.calRemainingMs / 1000);
          calButton.disabled = true;
          calButton.textContent = `Calibrate (Wait ${seconds}s)`;
        } else {
          calButton.disabled = false;
          calButton.textContent = 'Calibrate (Use Current Air)';
        }
        if (data.fw !== undefined) document.getElementById('fw').textContent = data.fw || '--';
      } catch (e) {
      }
    }

    async function setABC(enabled) {
      try {
        const body = new URLSearchParams();
        body.set('enabled', enabled ? 'true' : 'false');
        const response = await fetch('/abc', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body
        });
        if (!response.ok) {
          setStatus('Failed to update ABC');
          return;
        }
        const data = await response.json();
        document.getElementById('abc').textContent = data.abc ? 'ON' : 'OFF';
        updateABCButtons(!!data.abc);
        setStatus('ABC updated');
      } catch (e) {
        setStatus('Failed to update ABC');
      }
    }

    async function refreshABC() {
      try {
        const response = await fetch('/abc/refresh');
        if (!response.ok) {
          setStatus('Failed to refresh ABC');
          return;
        }
        const data = await response.json();
        document.getElementById('abc').textContent = data.abc ? 'ON' : 'OFF';
        updateABCButtons(!!data.abc);
        setStatus('ABC refreshed');
      } catch (e) {
        setStatus('Failed to refresh ABC');
      }
    }

    async function calibrate() {
      if (!confirm('Calibrate using current air? This sets zero to ~400ppm.')) return;
      try {
        const response = await fetch('/calibrate', { method: 'POST' });
        if (!response.ok) {
          if (response.status === 429) {
            const data = await response.json();
            if (data && data.retryAfterMs) {
              const seconds = Math.ceil(data.retryAfterMs / 1000);
              setStatus(`Please wait ${seconds}s before calibrating again`);
              return;
            }
          }
          setStatus('Calibration failed');
          return;
        }
        setStatus('Calibration requested');
      } catch (e) {
        setStatus('Calibration failed');
      }
    }

    async function saveThreshold() {
      const value = parseInt(thresholdInput.value, 10);
      if (isNaN(value)) {
        setStatus('Invalid threshold');
        return;
      }
      try {
        const body = new URLSearchParams();
        body.set('value', String(value));
        const response = await fetch('/threshold', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body
        });
        if (!response.ok) {
          setStatus('Failed to save threshold');
          return;
        }
        const data = await response.json();
        syncThreshold(data.threshold);
        setStatus('Threshold saved');
      } catch (e) {
        setStatus('Failed to save threshold');
      }
    }

    async function saveRange() {
      const value = parseInt(rangeInput.value, 10);
      if (isNaN(value)) {
        setStatus('Invalid range');
        return;
      }
      try {
        const body = new URLSearchParams();
        body.set('value', String(value));
        const response = await fetch('/range', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body
        });
        if (!response.ok) {
          setStatus('Failed to save range');
          return;
        }
        const data = await response.json();
        if (data.range !== undefined) {
          syncRange(data.range);
        }
        setStatus('Range saved');
      } catch (e) {
        setStatus('Failed to save range');
      }
    }

    async function loadBackground() {
      try {
        const response = await fetch('/background');
        if (!response.ok) return;
        const data = await response.json();
        if (data.background !== undefined) {
          document.getElementById('bgco2').textContent = data.background;
        }
      } catch (e) {
      }
    }

    async function resetDevice() {
      if (!confirm('Restart the ESP MCU now?')) return;
      try {
        const response = await fetch('/reset', { method: 'POST' });
        if (!response.ok) {
          setStatus('Reset failed');
          return;
        }
        setStatus('Restarting...');
      } catch (e) {
        setStatus('Reset failed');
      }
    }

    loadBackground();
    updateData();
    setInterval(updateData, 2000);
  </script>
</body>
</html>
)rawliteral";
