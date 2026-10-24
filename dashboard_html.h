#pragma once
#include <Arduino.h>

  // ==============================================================================
  // ARES ROVER — High-Tech Web Teleoperation Dashboard (Embedded PROGMEM HTML/CSS/JS)
  // No internet connection required. Stored directly in the ESP32 internal flash memory.
  // ==============================================================================

  const char INDEX_HTML[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">

  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>ARES // Rescue Rover Control</title>
    <style>
      :root {
        --bg-dark: #0b0f19;
        --panel-bg: rgba(18, 24, 38, 0.85);
        --panel-border: #1e293b;
        --accent-cyan: #00f0ff;
        --accent-blue: #0284c7;
        --accent-amber: #f59e0b;
        --accent-red: #ef4444;
        --accent-green: #10b981;
        --text-main: #f1f5f9;
        --text-muted: #94a3b8;
      }

      * {
        box-sizing: border-box;
        margin: 0;
        padding: 0;
        user-select: none;
        -webkit-user-select: none;
        -webkit-touch-callout: none;
      }

      body {
        background-color: var(--bg-dark);
        background-image: radial-gradient(circle at 50% 0%, #1e293b 0%, #0b0f19 75%);
        color: var(--text-main);
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", monospace, sans-serif;
        min-height: 100vh;
        display: flex;
        flex-direction: column;
        overflow-x: hidden;
        padding-bottom: 20px;
      }

      /* Top HUD Navigation Bar */
      header {
        background: rgba(11, 15, 25, 0.95);
        border-bottom: 1px solid var(--panel-border);
        padding: 12px 20px;
        display: flex;
        justify-content: space-between;
        align-items: center;
        position: sticky;
        top: 0;
        z-index: 100;
        backdrop-filter: blur(8px);
      }

      .brand {
        display: flex;
        align-items: center;
        gap: 12px;
      }

      .brand-logo {
        width: 28px;
        height: 28px;
        border: 2px solid var(--accent-cyan);
        border-radius: 6px;
        display: flex;
        align-items: center;
        justify-content: center;
        color: var(--accent-cyan);
        font-weight: 900;
        font-size: 14px;
        box-shadow: 0 0 10px rgba(0, 240, 255, 0.3);
      }

      .brand-title {
        font-size: 16px;
        font-weight: 800;
        letter-spacing: 2px;
        color: #fff;
      }

      .brand-sub {
        font-size: 10px;
        color: var(--accent-cyan);
        letter-spacing: 1px;
      }

      .hud-pills {
        display: flex;
        align-items: center;
        gap: 10px;
      }

      .pill {
        background: var(--panel-bg);
        border: 1px solid var(--panel-border);
        border-radius: 20px;
        padding: 4px 10px;
        font-size: 11px;
        font-weight: 600;
        display: flex;
        align-items: center;
        gap: 6px;
      }

      .status-dot {
        width: 8px;
        height: 8px;
        border-radius: 50%;
        background: var(--accent-amber);
        transition: background 0.3s;
      }

      .status-dot.online {
        background: var(--accent-green);
        box-shadow: 0 0 8px var(--accent-green);
      }

      .status-dot.offline {
        background: var(--accent-red);
        box-shadow: 0 0 8px var(--accent-red);
      }

      /* Main Container */
      main {
        flex: 1;
        max-width: 1200px;
        margin: 0 auto;
        width: 100%;
        padding: 16px;
        display: grid;
        grid-template-columns: 1fr 1fr;
        gap: 16px;
      }

      @media (max-width: 850px) {
        main {
          grid-template-columns: 1fr;
        }
      }

      /* Generic Panel Card */
      .card {
        background: var(--panel-bg);
        border: 1px solid var(--panel-border);
        border-radius: 12px;
        padding: 16px;
        box-shadow: 0 8px 24px rgba(0, 0, 0, 0.35);
        position: relative;
      }

      .card-title {
        font-size: 12px;
        font-weight: 700;
        letter-spacing: 1.5px;
        color: var(--text-muted);
        text-transform: uppercase;
        margin-bottom: 14px;
        display: flex;
        justify-content: space-between;
        align-items: center;
      }

      /* Telemetry HUD Grid */
      .telemetry-grid {
        display: grid;
        grid-template-columns: repeat(3, 1fr);
        gap: 10px;
        margin-bottom: 16px;
      }

      .telem-box {
        background: rgba(11, 15, 25, 0.6);
        border: 1px solid #1e293b;
        border-radius: 8px;
        padding: 10px;
        text-align: center;
      }

      .telem-label {
        font-size: 10px;
        color: var(--text-muted);
        text-transform: uppercase;
        letter-spacing: 0.5px;
      }

      .telem-val {
        font-size: 18px;
        font-weight: 800;
        color: var(--accent-cyan);
        margin-top: 4px;
        font-family: monospace;
      }

      /* 6-Wheel Chassis Visualizer */
      .chassis-view {
        position: relative;
        width: 200px;
        height: 190px;
        margin: 10px auto;
        background: rgba(15, 23, 42, 0.7);
        border: 2px dashed #334155;
        border-radius: 16px;
        display: flex;
        align-items: center;
        justify-content: center;
      }

      .rover-core {
        width: 90px;
        height: 130px;
        background: #1e293b;
        border: 2px solid var(--accent-cyan);
        border-radius: 8px;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        color: var(--accent-cyan);
        font-size: 11px;
        font-weight: 800;
        box-shadow: 0 0 15px rgba(0, 240, 255, 0.15);
        position: relative;
      }

      .rover-arrow {
        font-size: 18px;
        margin-bottom: 4px;
        transition: transform 0.3s;
      }

      .wheel {
        position: absolute;
        width: 24px;
        height: 40px;
        background: #334155;
        border: 2px solid #64748b;
        border-radius: 6px;
        transition: all 0.2s ease;
      }

      .wheel.active-fwd {
        border-color: var(--accent-cyan);
        background: rgba(0, 240, 255, 0.4);
        box-shadow: 0 0 10px var(--accent-cyan);
      }

      .wheel.active-rev {
        border-color: var(--accent-amber);
        background: rgba(245, 158, 11, 0.4);
        box-shadow: 0 0 10px var(--accent-amber);
      }

      /* Wheel Placements (6-Wheel Architecture) */
      .w-m1 {
        top: 15px;
        left: -14px;
      }

      /* Front Left */
      .w-m2 {
        top: 15px;
        right: -14px;
      }

      /* Front Right */
      .w-m3 {
        top: 75px;
        left: -14px;
      }

      /* Middle Left */
      .w-m4 {
        top: 75px;
        right: -14px;
      }

      /* Middle Right */
      .w-m5 {
        bottom: 15px;
        left: -14px;
      }

      /* Rear Left */
      .w-m6 {
        bottom: 15px;
        right: -14px;
      }

      /* Rear Right */

      /* Touch Control Deck */
      .control-deck {
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        gap: 12px;
        margin: 10px 0;
      }

      .dpad-row {
        display: flex;
        gap: 12px;
        justify-content: center;
      }

      .btn-drive {
        width: 72px;
        height: 72px;
        background: #1e293b;
        border: 2px solid #334155;
        border-radius: 14px;
        color: #fff;
        font-size: 24px;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        cursor: pointer;
        touch-action: manipulation;
        transition: all 0.1s ease;
        box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
      }

      .btn-drive span {
        font-size: 9px;
        font-weight: 700;
        color: var(--text-muted);
        margin-top: 2px;
        letter-spacing: 0.5px;
      }

      .btn-drive:active,
      .btn-drive.active {
        transform: scale(0.93);
        border-color: var(--accent-cyan);
        background: rgba(0, 240, 255, 0.2);
        box-shadow: 0 0 16px rgba(0, 240, 255, 0.5);
        color: var(--accent-cyan);
      }

      .btn-stop {
        width: 72px;
        height: 72px;
        background: rgba(239, 68, 68, 0.15);
        border: 2px solid var(--accent-red);
        border-radius: 14px;
        color: var(--accent-red);
        font-weight: 900;
        font-size: 13px;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        cursor: pointer;
        box-shadow: 0 0 12px rgba(239, 68, 68, 0.25);
      }

      .btn-stop:active {
        background: var(--accent-red);
        color: #fff;
        transform: scale(0.93);
      }

      /* Speed Slider & Presets */
      .slider-container {
        margin-top: 14px;
        background: rgba(11, 15, 25, 0.6);
        padding: 14px;
        border-radius: 10px;
        border: 1px solid #1e293b;
      }

      .slider-header {
        display: flex;
        justify-content: space-between;
        font-size: 12px;
        font-weight: 700;
        margin-bottom: 8px;
      }

      .speed-slider {
        width: 100%;
        height: 8px;
        border-radius: 4px;
        background: #334155;
        outline: none;
        -webkit-appearance: none;
      }

      .speed-slider::-webkit-slider-thumb {
        -webkit-appearance: none;
        appearance: none;
        width: 24px;
        height: 24px;
        border-radius: 50%;
        background: var(--accent-cyan);
        cursor: pointer;
        box-shadow: 0 0 10px var(--accent-cyan);
      }

      .preset-pills {
        display: flex;
        gap: 8px;
        margin-top: 10px;
      }

      .preset-btn {
        flex: 1;
        padding: 8px;
        background: #1e293b;
        border: 1px solid #334155;
        border-radius: 6px;
        color: var(--text-main);
        font-size: 11px;
        font-weight: 700;
        cursor: pointer;
        transition: all 0.2s;
      }

      .preset-btn:active,
      .preset-btn.active {
        background: var(--accent-cyan);
        color: #0b0f19;
        border-color: var(--accent-cyan);
      }

      /* Auxiliary Systems Grid */
      .aux-grid {
        display: grid;
        grid-template-columns: 1fr 1fr;
        gap: 10px;
        margin-top: 12px;
      }

      .aux-btn {
        padding: 12px;
        background: #1e293b;
        border: 1px solid #334155;
        border-radius: 8px;
        color: #fff;
        font-size: 12px;
        font-weight: 700;
        display: flex;
        align-items: center;
        justify-content: center;
        gap: 8px;
        cursor: pointer;
        transition: all 0.2s;
      }

      .aux-btn.active {
        border-color: var(--accent-amber);
        background: rgba(245, 158, 11, 0.2);
        color: var(--accent-amber);
      }

      /* Live Terminal Console */
      .console-box {
        background: #050811;
        border: 1px solid #1e293b;
        border-radius: 8px;
        padding: 10px;
        height: 90px;
        overflow-y: auto;
        font-family: monospace;
        font-size: 11px;
        color: #38bdf8;
        margin-top: 10px;
      }

      .console-line {
        margin-bottom: 3px;
      }

      .console-line.err {
        color: var(--accent-red);
      }

      .console-line.warn {
        color: var(--accent-amber);
      }

      /* Keyboard Teleop Hint */
      .kb-hint {
        font-size: 10px;
        color: var(--text-muted);
        text-align: center;
        margin-top: 8px;
      }
    </style>
  </head>

  <body>

    <!-- Top HUD Bar -->
    <header>
      <div class="brand">
        <div class="brand-logo">A</div>
        <div>
          <div class="brand-title">ARES ROVER</div>
          <div class="brand-sub">6-WHEEL RESCUE PLATFORM</div>
        </div>
      </div>
      <div class="hud-pills">
        <div class="pill">
          <div class="status-dot online" id="statusDot"></div>
          <span id="connLabel">CONNECTED</span>
        </div>
        <div class="pill">
          <span>⚡</span>
          <span id="battVal">11.8 V (SIM)</span>
        </div>
      </div>
    </header>

    <main>
      <!-- LEFT PANEL: Telemetry, Chassis Visualizer & Sensors -->
      <section class="card">
        <div class="card-title">
          <span>VEHICLE TELEMETRY</span>
          <span id="uptimeVal">UP: 00:00:00</span>
        </div>

        <div class="telemetry-grid">
          <div class="telem-box">
            <div class="telem-label">STATE</div>
            <div class="telem-val" id="motionState" style="font-size: 14px;">STOPPED</div>
          </div>
          <div class="telem-box">
            <div class="telem-label">SPEED</div>
            <div class="telem-val" id="speedDisp">63%</div>
          </div>
          <div class="telem-box">
            <div class="telem-label">FAILSAFE</div>
            <div class="telem-val" id="watchdogDisp" style="color: var(--accent-green); font-size: 14px;">ACTIVE</div>
          </div>
        </div>

        <!-- 6-Wheel Dynamic Chassis Map -->
        <div class="chassis-view">
          <div class="wheel w-m1" id="wheel-m1" title="Front Left"></div>
          <div class="wheel w-m2" id="wheel-m2" title="Front Right"></div>
          <div class="wheel w-m3" id="wheel-m3" title="Middle Left"></div>
          <div class="wheel w-m4" id="wheel-m4" title="Middle Right"></div>
          <div class="wheel w-m5" id="wheel-m5" title="Rear Left"></div>
          <div class="wheel w-m6" id="wheel-m6" title="Rear Right"></div>
          <div class="rover-core">
            <div class="rover-arrow" id="roverArrow">▲</div>
            <span>ARES-6W</span>
          </div>
        </div>

        <!-- Live Event Console -->
        <div class="card-title" style="margin-top: 14px; margin-bottom: 6px;">
          <span>TELEMETRY LOG</span>
          <span id="pingVal">PING: -- ms</span>
        </div>
        <div class="console-box" id="consoleBox">
          <div class="console-line">[SYS] ARES Rover Teleoperation online.</div>
        </div>
      </section>

      <!-- RIGHT PANEL: Driving Deck, Speed Controls, Rescue Functions -->
      <section class="card">
        <div class="card-title">
          <span>MOTION COMMAND DECK</span>
          <span>SKID-STEER 6WD</span>
        </div>

        <!-- D-Pad Controls (Touch + Click) -->
        <div class="control-deck">
          <!-- Row 1: Pivot Left, Forward, Pivot Right -->
          <div class="dpad-row">
            <button class="btn-drive" id="btn-q" data-cmd="q">↰<span>PIVOT L</span></button>
            <button class="btn-drive" id="btn-w" data-cmd="w">▲<span>FORWARD</span></button>
            <button class="btn-drive" id="btn-e" data-cmd="e">↱<span>PIVOT R</span></button>
          </div>

          <!-- Row 2: Skid Spin Left, STOP, Skid Spin Right -->
          <div class="dpad-row">
            <button class="btn-drive" id="btn-a" data-cmd="a">↺<span>SPIN L</span></button>
            <button class="btn-stop" id="btn-x" data-cmd="x">■<span>STOP</span></button>
            <button class="btn-drive" id="btn-d" data-cmd="d">↻<span>SPIN R</span></button>
          </div>

          <!-- Row 3: Brake, Reverse, Self-Test -->
          <div class="dpad-row">
            <button class="btn-drive" id="btn-b" data-cmd="b"
              style="border-color: #6366f1;">⚡<span>BRAKE</span></button>
            <button class="btn-drive" id="btn-s" data-cmd="s">▼<span>REVERSE</span></button>
            <button class="btn-drive" id="btn-t" data-cmd="t" style="border-color: #06b6d4;">⚙<span>TEST</span></button>
          </div>
        </div>

        <div class="kb-hint">Keyboard: [W, A, S, D] to drive • [Q, E] pivot • [SPACE] stop</div>

        <!-- Continuous Speed Control -->
        <div class="slider-container">
          <div class="slider-header">
            <span>MOTOR SPEED PWM</span>
            <span id="sliderValLabel">160 / 255 (63%)</span>
          </div>
          <input type="range" min="90" max="255" value="160" class="speed-slider" id="speedSlider">
          <div class="preset-pills">
            <button class="preset-btn" onclick="setSpeedPreset(100, this)">CRAWL (39%)</button>
            <button class="preset-btn active" onclick="setSpeedPreset(160, this)">CRUISE (63%)</button>
            <button class="preset-btn" onclick="setSpeedPreset(255, this)">TURBO (100%)</button>
          </div>
        </div>

        <!-- Auxiliary Rescue Subsystems -->
        <div class="aux-grid">
          <button class="aux-btn" id="btnLight" onclick="toggleLight()">
            💡 <span>LIGHTS: OFF [SIM]</span>
          </button>
          <button class="aux-btn" id="btnBuzzer" onclick="toggleBuzzer()">
            🚨 <span>BEACON: OFF [SIM]</span>
          </button>
        </div>
      </section>
    </main>

    <script>
      // System State Variables
      let currentSpeed = 160;
      let activeCmd = null;
      let driveInterval = null;
      let lastPing = 0;
      let lightState = false;
      let buzzerState = false;

      const consoleBox = document.getElementById('consoleBox');

      function logMsg(msg, type = '') {
        const line = document.createElement('div');
        line.className = 'console-line ' + type;
        const time = new Date().toTimeString().split(' ')[0];
        line.textContent = `[${time}] ${msg}`;
        consoleBox.appendChild(line);
        consoleBox.scrollTop = consoleBox.scrollHeight;
      }

      // Fast command dispatch to ESP32
      function sendCommand(cmd, spd = currentSpeed) {
        const url = `/cmd?c=${encodeURIComponent(cmd)}&spd=${spd}`;
        fetch(url)
          .then(res => res.json())
          .then(data => {
            updateUIState(data);
          })
          .catch(err => {
            // Connection issue handled by telemetry poller
          });
      }

      // Press & Hold Drive Loop (Prevents safety watchdog timeout)
      function startDriving(cmd) {
        if (activeCmd === cmd) return;
        activeCmd = cmd;
        sendCommand(cmd);

        // Send heartbeat drive packet every 250ms while button is held
        clearInterval(driveInterval);
        driveInterval = setInterval(() => {
          if (activeCmd) {
            sendCommand(activeCmd);
          }
        }, 250);
      }

      function stopDriving() {
        if (!activeCmd) return;
        activeCmd = null;
        clearInterval(driveInterval);
        sendCommand('x'); // Emergency stop
      }

      // Attach Touch and Mouse Handlers for D-Pad
      document.querySelectorAll('.btn-drive, .btn-stop').forEach(btn => {
        const cmd = btn.getAttribute('data-cmd');

        // Desktop Mouse Handlers
        btn.addEventListener('mousedown', (e) => {
          e.preventDefault();
          startDriving(cmd);
        });
        btn.addEventListener('mouseup', (e) => {
          e.preventDefault();
          stopDriving();
        });
        btn.addEventListener('mouseleave', () => {
          if (activeCmd === cmd) stopDriving();
        });

        // Mobile Touch Handlers
        btn.addEventListener('touchstart', (e) => {
          e.preventDefault();
          btn.classList.add('active');
          startDriving(cmd);
        }, { passive: false });

        btn.addEventListener('touchend', (e) => {
          e.preventDefault();
          btn.classList.remove('active');
          stopDriving();
        }, { passive: false });

        btn.addEventListener('touchcancel', () => {
          btn.classList.remove('active');
          stopDriving();
        });
      });

      // Keyboard Teleoperation
      const keyMap = {
        'w': 'w', 'ArrowUp': 'w',
        's': 's', 'ArrowDown': 's',
        'a': 'a', 'ArrowLeft': 'a',
        'd': 'd', 'ArrowRight': 'd',
        'q': 'q',
        'e': 'e',
        'b': 'b',
        ' ': 'x'
      };

      window.addEventListener('keydown', (e) => {
        if (e.repeat) return;
        const key = e.key;
        if (keyMap[key]) {
          e.preventDefault();
          const cmd = keyMap[key];
          const btn = document.getElementById('btn-' + cmd);
          if (btn) btn.classList.add('active');
          startDriving(cmd);
        }
      });

      window.addEventListener('keyup', (e) => {
        const key = e.key;
        if (keyMap[key]) {
          const cmd = keyMap[key];
          const btn = document.getElementById('btn-' + cmd);
          if (btn) btn.classList.remove('active');
          stopDriving();
        }
      });

      // Speed Slider Handler
      const speedSlider = document.getElementById('speedSlider');
      const sliderLabel = document.getElementById('sliderValLabel');

      speedSlider.addEventListener('input', (e) => {
        currentSpeed = parseInt(e.target.value);
        const pct = Math.round((currentSpeed / 255) * 100);
        sliderLabel.textContent = `${currentSpeed} / 255 (${pct}%)`;
        document.querySelectorAll('.preset-btn').forEach(b => b.classList.remove('active'));
        // Send speed update
        fetch(`/speed?val=${currentSpeed}`);
      });

      function setSpeedPreset(val, btnElem) {
        currentSpeed = val;
        speedSlider.value = val;
        const pct = Math.round((val / 255) * 100);
        sliderLabel.textContent = `${val} / 255 (${pct}%)`;
        document.querySelectorAll('.preset-btn').forEach(b => b.classList.remove('active'));
        btnElem.classList.add('active');
        fetch(`/speed?val=${currentSpeed}`);
        logMsg(`Speed preset set to ${pct}%`);
      }

      // Auxiliary Light & Siren Controls
      function toggleLight() {
        lightState = !lightState;
        const btn = document.getElementById('btnLight');
        btn.classList.toggle('active', lightState);
        fetch(`/aux?item=light&state=${lightState ? 1 : 0}`);
        logMsg(`Lights command sent: ${lightState ? 'ON' : 'OFF'}`);
      }

      function toggleBuzzer() {
        buzzerState = !buzzerState;
        const btn = document.getElementById('btnBuzzer');
        btn.classList.toggle('active', buzzerState);
        fetch(`/aux?item=beacon&state=${buzzerState ? 1 : 0}`);
        logMsg(`Beacon command sent: ${buzzerState ? 'ON' : 'OFF'}`);
      }

      // Telemetry Sync & Dynamic UI Animation
      function updateUIState(data) {
        if (!data) return;
        document.getElementById('motionState').textContent = data.state || 'UNKNOWN';
        document.getElementById('speedDisp').textContent = Math.round(((data.speed || currentSpeed) / 255) * 100) + '%';

        // Update Rover Arrow
        const arrow = document.getElementById('roverArrow');
        if (data.state === 'FORWARD') arrow.textContent = '▲';
        else if (data.state === 'REVERSE') arrow.textContent = '▼';
        else if (data.state === 'ROTATE_LEFT' || data.state === 'TURN_LEFT') arrow.textContent = '↺';
        else if (data.state === 'ROTATE_RIGHT' || data.state === 'TURN_RIGHT') arrow.textContent = '↻';
        else arrow.textContent = '■';

        // Update 6-Wheel Dynamic Map
        const leftActiveFwd = (data.state === 'FORWARD' || data.state === 'TURN_RIGHT' || data.state === 'ROTATE_RIGHT');
        const leftActiveRev = (data.state === 'REVERSE' || data.state === 'ROTATE_LEFT');
        const rightActiveFwd = (data.state === 'FORWARD' || data.state === 'TURN_LEFT' || data.state === 'ROTATE_LEFT');
        const rightActiveRev = (data.state === 'REVERSE' || data.state === 'ROTATE_RIGHT');

        ['m1', 'm3', 'm5'].forEach(id => {
          const el = document.getElementById('wheel-' + id);
          el.className = 'wheel w-' + id + (leftActiveFwd ? ' active-fwd' : '') + (leftActiveRev ? ' active-rev' : '');
        });

        ['m2', 'm4', 'm6'].forEach(id => {
          const el = document.getElementById('wheel-' + id);
          el.className = 'wheel w-' + id + (rightActiveFwd ? ' active-fwd' : '') + (rightActiveRev ? ' active-rev' : '');
        });
      }

      // Background Telemetry Poller (300ms cycle)
      setInterval(() => {
        const t0 = performance.now();
        fetch('/status')
          .then(res => res.json())
          .then(data => {
            const t1 = performance.now();
            document.getElementById('pingVal').textContent = `PING: ${Math.round(t1 - t0)} ms`;
            document.getElementById('statusDot').className = 'status-dot online';
            document.getElementById('connLabel').textContent = 'CONNECTED';

            if (data.uptime !== undefined) {
              const sec = data.uptime;
              const h = String(Math.floor(sec / 3600)).padStart(2, '0');
              const m = String(Math.floor((sec % 3600) / 60)).padStart(2, '0');
              const s = String(sec % 60).padStart(2, '0');
              document.getElementById('uptimeVal').textContent = `UP: ${h}:${m}:${s}`;
            }

            if (data.batt !== undefined) {
              document.getElementById('battVal').textContent = data.batt.toFixed(1) + ' V' + (data.batt_sim ? ' (SIM)' : '');
            }

            if (data.light !== undefined) {
              lightState = data.light;
              const btn = document.getElementById('btnLight');
              btn.classList.toggle('active', lightState);
              btn.querySelector('span').textContent = `LIGHTS: ${lightState ? 'ON' : 'OFF'}${data.light_hw ? '' : ' [SIM]'}`;
            }

            if (data.beacon !== undefined) {
              buzzerState = data.beacon;
              const btn = document.getElementById('btnBuzzer');
              btn.classList.toggle('active', buzzerState);
              btn.querySelector('span').textContent = `BEACON: ${buzzerState ? 'ON' : 'OFF'}${data.beacon_hw ? '' : ' [SIM]'}`;
            }

            updateUIState(data);
          })
          .catch(err => {
            document.getElementById('statusDot').className = 'status-dot offline';
            document.getElementById('connLabel').textContent = 'RECONNECTING';
          });
      }, 300);
    </script>
  </body>

  </html>
  )rawliteral";