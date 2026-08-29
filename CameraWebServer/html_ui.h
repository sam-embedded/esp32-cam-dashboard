#pragma once
#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>ESP32-CAM Pro Dashboard</title>
  <style>
    :root {
      --bg: #090d16;
      --card-bg: rgba(22, 30, 49, 0.85);
      --card-border: rgba(255, 255, 255, 0.09);
      --text: #f1f5f9;
      --text-muted: #94a3b8;
      --accent: #0284c7;
      --accent-hover: #0369a1;
      --accent-glow: rgba(2, 132, 199, 0.35);
      --success: #10b981;
      --danger: #ef4444;
      --warning: #f59e0b;
      --drawer-w: 320px;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; -webkit-tap-highlight-color: transparent; }
    body { background: var(--bg); color: var(--text); min-height: 100vh; overflow-x: hidden; display: flex; flex-direction: column; }
    
    /* Top Bar - Mobile Optimized, No Overlapping */
    header {
      background: rgba(15, 23, 42, 0.92);
      backdrop-filter: blur(16px);
      -webkit-backdrop-filter: blur(16px);
      padding: 0.5rem 0.75rem;
      display: flex;
      justify-content: space-between;
      align-items: center;
      border-bottom: 1px solid var(--card-border);
      position: sticky;
      top: 0;
      z-index: 100;
      gap: 0.5rem;
      height: 54px;
    }
    .header-left { display: flex; align-items: center; gap: 0.5rem; flex-shrink: 0; }
    .btn-hamburger {
      width: 40px;
      height: 40px;
      display: flex;
      align-items: center;
      justify-content: center;
      background: rgba(255, 255, 255, 0.08);
      border: 1px solid var(--card-border);
      border-radius: 0.5rem;
      color: #38bdf8;
      font-size: 1.35rem;
      cursor: pointer;
      transition: all 0.2s;
    }
    .btn-hamburger:hover { background: var(--accent-glow); border-color: #38bdf8; }
    .brand-title { font-weight: 700; font-size: 1.05rem; letter-spacing: -0.02em; white-space: nowrap; color: #fff; display: flex; align-items: center; gap: 0.35rem; }
    
    .header-right { display: flex; align-items: center; gap: 0.35rem; flex-shrink: 0; }
    
    /* Buttons */
    .btn {
      background: var(--accent);
      color: white;
      border: none;
      padding: 0.45rem 0.75rem;
      border-radius: 0.5rem;
      font-size: 0.85rem;
      font-weight: 500;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 0.35rem;
      transition: all 0.15s ease;
      min-height: 38px;
    }
    .btn:hover { background: var(--accent-hover); box-shadow: 0 0 12px var(--accent-glow); }
    .btn-icon { width: 38px; height: 38px; padding: 0; border-radius: 0.5rem; background: var(--card-bg); border: 1px solid var(--card-border); color: var(--text); font-size: 1.1rem; }
    .btn-icon:hover { background: rgba(255,255,255,0.12); }
    .btn-danger { background: var(--danger); }
    .btn-danger:hover { background: #dc2626; }
    .btn-success { background: var(--success); }
    .btn-success:hover { background: #059669; }
    .btn-full { width: 100%; margin-top: 0.5rem; }
    .btn-sm { padding: 0.25rem 0.5rem; font-size: 0.75rem; min-height: 30px; }

    /* Main Viewport */
    .main-content {
      flex: 1;
      display: flex;
      flex-direction: column;
      height: calc(100vh - 54px);
      background: #000;
      position: relative;
    }
    .viewport {
      position: relative;
      flex: 1;
      width: 100%;
      background: #000;
      display: flex;
      justify-content: center;
      align-items: center;
      overflow: hidden;
    }
    #stream-img {
      max-width: 100%;
      max-height: 100%;
      object-fit: contain;
      user-select: none;
    }
    
    /* Overlay HUD */
    .stream-overlay {
      position: absolute;
      top: 0.65rem;
      left: 0.65rem;
      background: rgba(0, 0, 0, 0.75);
      backdrop-filter: blur(8px);
      padding: 0.35rem 0.65rem;
      border-radius: 0.5rem;
      border: 1px solid var(--card-border);
      display: flex;
      align-items: center;
      gap: 0.45rem;
      font-size: 0.76rem;
      pointer-events: none;
      z-index: 10;
    }
    .live-dot { width: 8px; height: 8px; border-radius: 50%; background: var(--danger); transition: background 0.3s; flex-shrink: 0; }
    .live-dot.active { background: var(--success); box-shadow: 0 0 8px var(--success); }

    /* Status Bar Below Live Stream */
    .stream-status-bar {
      background: rgba(15, 23, 42, 0.92);
      border-top: 1px solid var(--card-border);
      padding: 0.5rem 0.85rem;
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 0.5rem;
      flex-wrap: wrap;
      z-index: 20;
      font-size: 0.82rem;
    }
    .status-group { display: flex; align-items: center; gap: 0.75rem; flex-wrap: wrap; }
    .status-badge {
      display: inline-flex;
      align-items: center;
      gap: 0.35rem;
      background: rgba(255,255,255,0.05);
      padding: 0.25rem 0.6rem;
      border-radius: 0.4rem;
      border: 1px solid var(--card-border);
      white-space: nowrap;
      font-weight: 500;
    }

    /* Left Navigation Drawer */
    .nav-backdrop {
      position: fixed;
      inset: 0;
      background: rgba(0,0,0,0.65);
      backdrop-filter: blur(4px);
      z-index: 200;
      opacity: 0;
      pointer-events: none;
      transition: opacity 0.3s ease;
    }
    .nav-backdrop.open { opacity: 1; pointer-events: auto; }
    .nav-drawer {
      position: fixed;
      top: 0;
      left: calc(-1 * var(--drawer-w));
      width: var(--drawer-w);
      height: 100vh;
      background: rgba(15, 23, 42, 0.98);
      backdrop-filter: blur(20px);
      -webkit-backdrop-filter: blur(20px);
      border-right: 1px solid var(--card-border);
      z-index: 201;
      display: flex;
      flex-direction: column;
      transition: left 0.3s cubic-bezier(0.4, 0, 0.2, 1);
      box-shadow: 10px 0 30px rgba(0,0,0,0.7);
    }
    .nav-drawer.open { left: 0; }
    .nav-header {
      padding: 1rem;
      border-bottom: 1px solid var(--card-border);
      display: flex;
      align-items: center;
      justify-content: space-between;
    }
    .nav-list { padding: 0.75rem; display: flex; flex-direction: column; gap: 0.4rem; overflow-y: auto; flex: 1; }
    .nav-item {
      display: flex;
      align-items: center;
      gap: 0.75rem;
      padding: 0.8rem 1rem;
      border-radius: 0.6rem;
      background: transparent;
      border: 1px solid transparent;
      color: var(--text);
      font-size: 0.92rem;
      font-weight: 500;
      cursor: pointer;
      text-align: left;
      transition: all 0.2s;
      width: 100%;
    }
    .nav-item:hover, .nav-item.active {
      background: rgba(2, 132, 199, 0.15);
      border-color: rgba(56, 189, 248, 0.3);
      color: #38bdf8;
    }
    .nav-item .icon { font-size: 1.25rem; width: 28px; text-align: center; }

    /* Right Camera Settings Drawer */
    .cam-drawer {
      position: fixed;
      top: 54px;
      right: -340px;
      width: 320px;
      height: calc(100vh - 54px);
      background: rgba(15, 23, 42, 0.95);
      backdrop-filter: blur(20px);
      -webkit-backdrop-filter: blur(20px);
      border-left: 1px solid var(--card-border);
      padding: 1rem;
      display: flex;
      flex-direction: column;
      gap: 0.85rem;
      overflow-y: auto;
      transition: right 0.3s cubic-bezier(0.4, 0, 0.2, 1);
      z-index: 90;
    }
    .cam-drawer.open { right: 0; }

    /* Controls & Forms */
    .ctrl-group { display: flex; flex-direction: column; gap: 0.35rem; }
    .ctrl-header { display: flex; justify-content: space-between; font-size: 0.8rem; color: var(--text-muted); }
    input[type=range] {
      width: 100%;
      height: 6px;
      background: rgba(255,255,255,0.15);
      border-radius: 3px;
      outline: none;
      -webkit-appearance: none;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 18px;
      height: 18px;
      border-radius: 50%;
      background: var(--accent);
      cursor: pointer;
      border: 2px solid #fff;
    }
    select, input[type=text], input[type=password], input[type=number] {
      width: 100%;
      background: rgba(0,0,0,0.35);
      border: 1px solid var(--card-border);
      color: var(--text);
      padding: 0.6rem 0.75rem;
      border-radius: 0.5rem;
      outline: none;
      font-size: 0.85rem;
    }
    select:focus, input:focus { border-color: var(--accent); }
    .toggle-row { display: flex; justify-content: space-between; align-items: center; padding: 0.35rem 0; }
    .switch { position: relative; display: inline-block; width: 44px; height: 24px; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer; inset: 0; background-color: rgba(255,255,255,0.15); transition: .3s; border-radius: 24px; }
    .slider:before { position: absolute; content: ""; height: 18px; width: 18px; left: 3px; bottom: 3px; background-color: white; transition: .3s; border-radius: 50%; }
    input:checked + .slider { background-color: var(--accent); }
    input:checked + .slider:before { transform: translateX(20px); }

    /* Modals */
    .modal-backdrop {
      position: fixed;
      inset: 0;
      background: rgba(0, 0, 0, 0.75);
      backdrop-filter: blur(8px);
      display: none;
      align-items: center;
      justify-content: center;
      z-index: 300;
      padding: 0.75rem;
    }
    .modal-backdrop.open { display: flex; }
    .modal {
      background: rgba(15, 23, 42, 0.98);
      border: 1px solid var(--card-border);
      border-radius: 0.85rem;
      width: 100%;
      max-width: 650px;
      max-height: 90vh;
      display: flex;
      flex-direction: column;
      overflow: hidden;
      box-shadow: 0 25px 50px -12px rgba(0,0,0,0.8);
      animation: modalIn 0.25s ease-out;
    }
    @keyframes modalIn { from { opacity:0; transform: scale(0.95); } to { opacity:1; transform: scale(1); } }
    .modal-header { padding: 1rem; border-bottom: 1px solid var(--card-border); display: flex; justify-content: space-between; align-items: center; }
    .modal-body { padding: 1rem; overflow-y: auto; display: flex; flex-direction: column; gap: 1rem; }
    
    /* File Manager Standalone Styles */
    .fm-modal { max-width: 850px; height: 85vh; }
    .fm-toolbar { display: flex; flex-wrap: wrap; gap: 0.5rem; align-items: center; justify-content: space-between; background: rgba(0,0,0,0.25); padding: 0.65rem 0.85rem; border-radius: 0.6rem; border: 1px solid var(--card-border); }
    .fm-breadcrumbs { display: flex; align-items: center; gap: 0.35rem; font-size: 0.82rem; overflow-x: auto; color: var(--text-muted); flex: 1; }
    .fm-crumb { cursor: pointer; color: #38bdf8; text-decoration: underline; white-space: nowrap; }
    .fm-crumb:hover { color: #fff; }
    .fm-storage { background: rgba(0,0,0,0.3); border-radius: 0.5rem; padding: 0.6rem 0.85rem; border: 1px solid var(--card-border); }
    .fm-progress-bar { height: 7px; background: rgba(255,255,255,0.1); border-radius: 4px; overflow: hidden; margin-top: 0.4rem; }
    .fm-progress-fill { height: 100%; background: linear-gradient(90deg, #0284c7, #10b981); width: 0%; transition: width 0.3s; }
    
    /* Batch Toolbar */
    .batch-bar { display: none; align-items: center; justify-content: space-between; background: rgba(2, 132, 199, 0.2); border: 1px solid rgba(56, 189, 248, 0.4); padding: 0.5rem 0.75rem; border-radius: 0.5rem; }
    .batch-bar.active { display: flex; }

    /* Grid vs List View */
    .fm-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(130px, 1fr)); gap: 0.75rem; padding: 0.25rem; }
    .fm-card {
      background: rgba(255,255,255,0.03);
      border: 1px solid var(--card-border);
      border-radius: 0.6rem;
      padding: 0.6rem;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 0.4rem;
      position: relative;
      transition: all 0.2s;
      cursor: pointer;
    }
    .fm-card:hover { background: rgba(255,255,255,0.08); border-color: rgba(56, 189, 248, 0.4); }
    .fm-card.selected { border-color: #38bdf8; background: rgba(2, 132, 199, 0.25); }
    .fm-card-chk { position: absolute; top: 6px; left: 6px; z-index: 5; }
    .fm-card-preview {
      width: 100%;
      height: 90px;
      border-radius: 0.4rem;
      background: rgba(0,0,0,0.4);
      display: flex;
      align-items: center;
      justify-content: center;
      overflow: hidden;
    }
    .fm-card-preview img { width: 100%; height: 100%; object-fit: cover; }
    .fm-card-icon { font-size: 2.2rem; }
    .fm-card-title { font-size: 0.76rem; width: 100%; text-align: center; word-break: break-all; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
    .fm-card-meta { font-size: 0.7rem; color: var(--text-muted); }
    .fm-card-actions { display: flex; gap: 0.3rem; margin-top: 0.2rem; }

    .fm-list { display: flex; flex-direction: column; gap: 0.35rem; }
    .fm-row {
      display: flex;
      align-items: center;
      gap: 0.6rem;
      background: rgba(255,255,255,0.03);
      border: 1px solid var(--card-border);
      padding: 0.5rem 0.75rem;
      border-radius: 0.5rem;
      transition: all 0.15s;
    }
    .fm-row:hover { background: rgba(255,255,255,0.07); }
    .fm-row.selected { border-color: #38bdf8; background: rgba(2, 132, 199, 0.2); }
    .fm-row-name { flex: 1; font-size: 0.82rem; word-break: break-all; }
    .fm-row-size { font-size: 0.75rem; color: var(--text-muted); width: 70px; text-align: right; }

    /* Toast */
    .toast-container { position: fixed; bottom: 1rem; right: 1rem; z-index: 999; display: flex; flex-direction: column; gap: 0.5rem; pointer-events: none; }
    .toast { background: rgba(15, 23, 42, 0.95); border: 1px solid var(--accent); color: white; padding: 0.65rem 1rem; border-radius: 0.5rem; font-size: 0.85rem; box-shadow: 0 10px 15px -3px rgba(0,0,0,0.5); pointer-events: auto; animation: toastIn 0.2s ease-out; }
    @keyframes toastIn { from { transform: translateY(20px); opacity: 0; } to { transform: translateY(0); opacity: 1; } }
  </style>
</head>
<body>

  <!-- Top Bar (Mobile Optimized, No Overlapping) -->
  <header>
    <div class="header-left">
      <button class="btn-hamburger" onclick="toggleNavDrawer()" title="Menu">☰</button>
      <div class="brand-title">📷 ESP32-CAM</div>
    </div>
    <div class="header-right">
      <button class="btn btn-icon" onclick="capturePhoto()" title="Capture Snapshot">📷</button>
      <button class="btn btn-icon" id="btn-flash" onclick="toggleFlash()" title="Flash Light">💡</button>
      <button class="btn btn-icon" onclick="openFileManager('/')" title="SD File Manager">📁</button>
      <button class="btn btn-icon" onclick="openModal('modal-settings')" title="Settings">⚙️</button>
    </div>
  </header>

  <!-- Main Viewport + Live Status Bar -->
  <div class="main-content">
    <div class="viewport">
      <div class="stream-overlay">
        <div class="live-dot" id="live-indicator"></div>
        <span id="stream-status">LIVE STREAM</span>
        <span style="color:var(--text-muted);">|</span>
        <span id="overlay-fps">25 FPS</span>
        <span style="color:var(--text-muted);">|</span>
        <span id="overlay-rssi">📶 -- dBm</span>
      </div>
      <img id="stream-img" src="" alt="Stream Viewport">
    </div>

    <!-- Live Status Strip directly below the stream -->
    <div class="stream-status-bar">
      <div class="status-group">
        <div class="status-badge" style="color:#38bdf8;">
          <span class="live-dot" id="bar-live-dot"></span>
          <span id="bar-status">Streaming</span>
        </div>
        <div class="status-badge" id="bar-fps">⚡ 25 FPS</div>
        <div class="status-badge" id="bar-rssi">📶 WiFi: -- dBm</div>
      </div>
      <div class="status-group">
        <div class="status-badge" id="bar-time" style="color:#38bdf8;">🕒 --</div>
        <div class="status-badge" id="bar-uptime" style="color:#10b981;">⏱ Uptime: 0s</div>
        <div class="status-badge" id="bar-ip" style="color:var(--text-muted);">🌐 192.168.31.113</div>
      </div>
    </div>
  </div>

  <!-- Left Navigation Drawer -->
  <div class="nav-backdrop" id="nav-backdrop" onclick="closeNavDrawer()"></div>
  <aside class="nav-drawer" id="nav-drawer">
    <div class="nav-header">
      <div class="brand-title">⚙️ Control Center</div>
      <button class="btn btn-icon" onclick="closeNavDrawer()" style="width:32px;height:32px;">✕</button>
    </div>
    <div class="nav-list">
      <button class="nav-item" onclick="closeNavDrawer(); toggleCamDrawer()">
        <span class="icon">🎛️</span>
        <div>
          <div>1. Stream Settings</div>
          <div style="font-size:0.72rem;color:var(--text-muted);">FPS, Resolution & Sensors</div>
        </div>
      </button>
      <button class="nav-item" onclick="closeNavDrawer(); openFileManager('/')">
        <span class="icon">📁</span>
        <div>
          <div>2. SD File Manager</div>
          <div style="font-size:0.72rem;color:var(--text-muted);">Browse folders, Thumbnails, Batch delete</div>
        </div>
      </button>
      <button class="nav-item" onclick="closeNavDrawer(); openTelegramManager()">
        <span class="icon">🤖</span>
        <div>
          <div>3. Telegram Server</div>
          <div style="font-size:0.72rem;color:var(--text-muted);">Token, Chat IDs & Instant tests</div>
        </div>
      </button>
      <button class="nav-item" onclick="closeNavDrawer(); openModal('modal-settings')">
        <span class="icon">⚙️</span>
        <div>
          <div>4. System & Network</div>
          <div style="font-size:0.72rem;color:var(--text-muted);">WiFi, Hostname, OTA & Reset</div>
        </div>
      </button>
    </div>
    <div style="padding:1rem;border-top:1px solid var(--card-border);font-size:0.75rem;color:var(--text-muted);text-align:center;">
      ESP32-CAM 24/7 Monitoring Firmware
    </div>
  </aside>

  <!-- Right Slide-out Drawer: Stream & Camera Adjustments -->
  <aside class="cam-drawer" id="cam-drawer">
    <div style="display:flex;justify-content:space-between;align-items:center;padding-bottom:0.5rem;border-bottom:1px solid var(--card-border);">
      <h3 style="font-size:0.95rem;">🎛️ Stream & Sensor</h3>
      <button class="btn btn-icon" onclick="toggleCamDrawer()" style="width:30px;height:30px;">✕</button>
    </div>

    <!-- FPS Control -->
    <div class="ctrl-group">
      <div class="ctrl-header"><span>Stream FPS Pacing</span><span id="val-fps">25 FPS</span></div>
      <input type="range" min="1" max="30" value="25" id="rng-fps" onchange="updateControl('fps', this.value); document.getElementById('val-fps').innerText=this.value+' FPS'; document.getElementById('overlay-fps').innerText=this.value+' FPS'; document.getElementById('bar-fps').innerText='⚡ '+this.value+' FPS';">
    </div>

    <!-- Resolution -->
    <div class="ctrl-group">
      <div class="ctrl-header"><span>Resolution</span></div>
      <select id="sel-res" onchange="updateControl('framesize', this.value)">
        <option value="10">UXGA (1600x1200)</option>
        <option value="9">SXGA (1280x1024)</option>
        <option value="8">XGA (1024x768)</option>
        <option value="7">SVGA (800x600)</option>
        <option value="6" selected>VGA (640x480) - Real-time</option>
        <option value="5">CIF (400x296)</option>
        <option value="4">QVGA (320x240)</option>
      </select>
    </div>

    <!-- JPEG Quality -->
    <div class="ctrl-group">
      <div class="ctrl-header"><span>JPEG Quality</span><span id="val-quality">12</span></div>
      <input type="range" min="10" max="63" value="12" id="rng-quality" oninput="document.getElementById('val-quality').innerText=this.value" onchange="updateControl('quality', this.value)">
    </div>

    <!-- Brightness, Contrast, Saturation -->
    <div class="ctrl-group">
      <div class="ctrl-header"><span>Brightness</span><span id="val-brightness">0</span></div>
      <input type="range" min="-2" max="2" value="0" id="rng-brightness" oninput="document.getElementById('val-brightness').innerText=this.value" onchange="updateControl('brightness', this.value)">
    </div>
    <div class="ctrl-group">
      <div class="ctrl-header"><span>Contrast</span><span id="val-contrast">0</span></div>
      <input type="range" min="-2" max="2" value="0" id="rng-contrast" oninput="document.getElementById('val-contrast').innerText=this.value" onchange="updateControl('contrast', this.value)">
    </div>
    <div class="ctrl-group">
      <div class="ctrl-header"><span>Saturation</span><span id="val-saturation">0</span></div>
      <input type="range" min="-2" max="2" value="0" id="rng-saturation" oninput="document.getElementById('val-saturation').innerText=this.value" onchange="updateControl('saturation', this.value)">
    </div>

    <!-- Special Effects & White Balance -->
    <div class="ctrl-group">
      <div class="ctrl-header"><span>Special Effect</span></div>
      <select id="sel-effect" onchange="updateControl('special_effect', this.value)">
        <option value="0">No Effect</option>
        <option value="1">Negative</option>
        <option value="2">Grayscale</option>
        <option value="3">Red Tint</option>
        <option value="4">Green Tint</option>
        <option value="5">Blue Tint</option>
        <option value="6">Sepia</option>
      </select>
    </div>
    <div class="ctrl-group">
      <div class="ctrl-header"><span>White Balance</span></div>
      <select id="sel-wb" onchange="updateControl('wb_mode', this.value)">
        <option value="0">Auto</option>
        <option value="1">Sunny</option>
        <option value="2">Cloudy</option>
        <option value="3">Office</option>
        <option value="4">Home</option>
      </select>
    </div>

    <!-- Toggles -->
    <div class="toggle-row">
      <span style="font-size:0.85rem;">Vertical Flip</span>
      <label class="switch"><input type="checkbox" id="chk-vflip" onchange="updateControl('vflip', this.checked?1:0)"><span class="slider"></span></label>
    </div>
    <div class="toggle-row">
      <span style="font-size:0.85rem;">Horizontal Mirror</span>
      <label class="switch"><input type="checkbox" id="chk-hmirror" onchange="updateControl('hmirror', this.checked?1:0)"><span class="slider"></span></label>
    </div>

    <button class="btn btn-full btn-success" onclick="saveCameraDefaults()">💾 Save Stream Defaults</button>
  </aside>

  <!-- Standalone Full SD Card File Manager Modal -->
  <div class="modal-backdrop" id="modal-fm">
    <div class="modal fm-modal">
      <div class="modal-header">
        <div style="font-weight:700;font-size:1.05rem;display:flex;align-items:center;gap:0.4rem;">
          📁 SD Card File Explorer
        </div>
        <div style="display:flex;align-items:center;gap:0.4rem;">
          <button class="btn btn-sm btn-icon" id="btn-view-mode" onclick="toggleViewMode()" title="Toggle Grid/List">⊞</button>
          <button class="btn btn-sm btn-icon" onclick="openFileManager(currentFmPath)" title="Refresh">🔄</button>
          <button class="btn btn-icon" onclick="closeModal('modal-fm')" style="width:30px;height:30px;">✕</button>
        </div>
      </div>

      <div class="modal-body" style="gap:0.65rem;flex:1;">
        <!-- Storage usage -->
        <div class="fm-storage">
          <div style="display:flex;justify-content:space-between;font-size:0.78rem;">
            <span>Storage Usage</span>
            <span id="fm-storage-text">Loading...</span>
          </div>
          <div class="fm-progress-bar"><div class="fm-progress-fill" id="fm-progress-fill"></div></div>
        </div>

        <!-- Toolbar & Breadcrumbs -->
        <div class="fm-toolbar">
          <div class="fm-breadcrumbs" id="fm-breadcrumbs">
            <span class="fm-crumb" onclick="openFileManager('/')">📁 Root</span>
          </div>
          <div style="display:flex;gap:0.35rem;">
            <button class="btn btn-sm" id="btn-up-dir" onclick="navigateUpDir()" style="background:#334155;">⬆️ Up</button>
            <button class="btn btn-sm" onclick="selectAllFiles()" id="btn-select-all" style="background:#334155;">☑️ Select All</button>
          </div>
        </div>

        <!-- Batch Action Bar -->
        <div class="batch-bar" id="batch-bar">
          <span id="batch-count" style="font-size:0.85rem;font-weight:600;color:#38bdf8;">0 items selected</span>
          <div style="display:flex;gap:0.4rem;">
            <button class="btn btn-sm btn-danger" onclick="deleteSelectedFiles()">🗑️ Delete Selected</button>
            <button class="btn btn-sm" onclick="clearSelection()" style="background:#475569;">✕ Clear</button>
          </div>
        </div>

        <!-- File List / Grid Container -->
        <div id="fm-container" style="flex:1;overflow-y:auto;min-height:220px;">
          <div style="text-align:center;padding:2rem;color:var(--text-muted);">Loading files...</div>
        </div>

        <!-- Danger Zone -->
        <div style="display:flex;justify-content:space-between;align-items:center;padding-top:0.5rem;border-top:1px solid var(--card-border);">
          <span style="font-size:0.75rem;color:var(--text-muted);">Format deletes all photos & videos</span>
          <button class="btn btn-sm btn-danger" onclick="formatSDCard()">🧹 Format SD Card</button>
        </div>
      </div>
    </div>
  </div>

  <!-- Settings & Telegram Modal -->
  <div class="modal-backdrop" id="modal-settings">
    <div class="modal">
      <div class="modal-header">
        <div style="font-weight:700;font-size:1.05rem;">⚙️ System & Server Settings</div>
        <button class="btn btn-icon" onclick="closeModal('modal-settings')" style="width:30px;height:30px;">✕</button>
      </div>
      <div class="modal-body">
        <!-- Tabs -->
        <div style="display:flex;gap:0.4rem;border-bottom:1px solid var(--card-border);padding-bottom:0.5rem;overflow-x:auto;">
          <button class="btn btn-sm" id="tab-btn-tg" onclick="switchTab('tab-tg')" style="background:var(--accent);">🤖 Telegram</button>
          <button class="btn btn-sm" id="tab-btn-ntp" onclick="switchTab('tab-ntp')" style="background:#334155;">⏱️ NTP Time</button>
          <button class="btn btn-sm" id="tab-btn-sys" onclick="switchTab('tab-sys')" style="background:#334155;">📶 WiFi & Host</button>
          <button class="btn btn-sm" id="tab-btn-ota" onclick="switchTab('tab-ota')" style="background:#334155;">⬆️ OTA Flash</button>
        </div>

        <!-- Tab: Telegram Server Manager -->
        <div id="tab-tg" style="display:flex;flex-direction:column;gap:0.75rem;">
          <div class="ctrl-group">
            <div class="ctrl-header"><span>Bot Token</span></div>
            <input type="password" id="cfg-tg-token" placeholder="8967102688:AAHEieQC2...">
          </div>
          <div class="ctrl-group">
            <div class="ctrl-header"><span>Chat IDs (comma-separated)</span></div>
            <input type="text" id="cfg-tg-chat" placeholder="318862528, 987654321">
          </div>
          <button class="btn btn-full" onclick="saveSettings()">💾 Save Telegram Config</button>

          <div style="margin-top:0.5rem;padding:0.75rem;background:rgba(255,255,255,0.03);border:1px solid var(--card-border);border-radius:0.5rem;display:flex;flex-direction:column;gap:0.5rem;">
            <div style="font-size:0.85rem;font-weight:600;color:#38bdf8;">🧪 Live Bot & HTTPS Diagnostics</div>
            <div style="display:grid;grid-template-columns:1fr 1fr;gap:0.4rem;">
              <button class="btn btn-sm" style="background:#0284c7;" onclick="testRawHTTPS()">🔒 Test Raw HTTPS</button>
              <button class="btn btn-sm" style="background:#475569;" onclick="sendTelegramTest('msg')">✉️ Test Text Msg</button>
            </div>
            <button class="btn btn-sm" style="background:#475569;" onclick="sendTelegramTest('photo')">📸 Test Camera Snapshot</button>
            
            <div id="tg-diag-box" style="display:none;font-size:0.72rem;background:rgba(0,0,0,0.5);border:1px solid rgba(56,189,248,0.2);padding:0.5rem;border-radius:0.35rem;white-space:pre-wrap;font-family:monospace;color:#38bdf8;"></div>

            <div style="font-size:0.75rem;color:var(--text-muted);margin-top:0.25rem;">
              Commands available in bot: <code>/photo</code>, <code>/flash on</code>, <code>/flash off</code>, <code>/status</code>, <code>/help</code>
            </div>
          </div>
        </div>

        <!-- Tab: NTP Time Settings -->
        <div id="tab-ntp" style="display:none;flex-direction:column;gap:0.75rem;">
          <div style="padding:0.6rem;background:rgba(56,189,248,0.08);border:1px solid rgba(56,189,248,0.2);border-radius:0.4rem;display:flex;align-items:center;justify-content:space-between;">
            <span style="font-size:0.8rem;color:var(--text-muted);">ESP32 System Clock:</span>
            <span id="cfg-clock-display" style="font-size:0.85rem;font-weight:700;color:#38bdf8;">--</span>
          </div>

          <div class="ctrl-group">
            <div class="ctrl-header"><span>Primary NTP Server</span></div>
            <input type="text" id="cfg-ntp1" placeholder="pool.ntp.org">
          </div>
          <div class="ctrl-group">
            <div class="ctrl-header"><span>Secondary NTP Server</span></div>
            <input type="text" id="cfg-ntp2" placeholder="time.nist.gov">
          </div>
          <div class="ctrl-group">
            <div class="ctrl-header"><span>Timezone</span></div>
            <select id="cfg-ntp-offset">
              <option value="19800">UTC +05:30 (India Standard Time - IST)</option>
              <option value="0">UTC +00:00 (GMT / UTC - London)</option>
              <option value="3600">UTC +01:00 (CET - Paris, Berlin)</option>
              <option value="7200">UTC +02:00 (EET - Cairo, Athens)</option>
              <option value="10800">UTC +03:00 (MSK / Arabia - Moscow, Riyadh)</option>
              <option value="14400">UTC +04:00 (GST - Dubai)</option>
              <option value="21600">UTC +06:00 (BST - Dhaka)</option>
              <option value="25200">UTC +07:00 (ICT - Bangkok, Jakarta)</option>
              <option value="28800">UTC +08:00 (CST / SGT - Singapore, Beijing)</option>
              <option value="32400">UTC +09:00 (JST / KST - Tokyo, Seoul)</option>
              <option value="36000">UTC +10:00 (AEST - Sydney)</option>
              <option value="-18000">UTC -05:00 (EST - New York)</option>
              <option value="-21600">UTC -06:00 (CST - Chicago)</option>
              <option value="-25200">UTC -07:00 (MST - Denver)</option>
              <option value="-28800">UTC -08:00 (PST - Los Angeles)</option>
            </select>
          </div>
          <div class="toggle-row">
            <span style="font-size:0.85rem;">Daylight Saving Time (DST +1h)</span>
            <label class="switch"><input type="checkbox" id="chk-dst"><span class="slider"></span></label>
          </div>
          <button class="btn btn-full btn-success" onclick="saveSettings()">💾 Save & Sync Clock</button>
        </div>

        <!-- Tab: System & WiFi -->
        <div id="tab-sys" style="display:none;flex-direction:column;gap:0.75rem;">
          <div class="ctrl-group">
            <div class="ctrl-header"><span>Device mDNS Hostname</span></div>
            <input type="text" id="cfg-mdns" placeholder="esp32cam">
          </div>
          <div class="ctrl-group">
            <div class="ctrl-header"><span>WiFi SSID</span></div>
            <input type="text" id="cfg-ssid" placeholder="FTTH">
          </div>
          <div class="ctrl-group">
            <div class="ctrl-header"><span>WiFi Password</span></div>
            <input type="password" id="cfg-pass" placeholder="••••••••">
          </div>
          <button class="btn btn-full" onclick="saveSettings()">💾 Save & Connect</button>
          
          <div style="display:grid;grid-template-columns:1fr 1fr;gap:0.5rem;margin-top:0.5rem;">
            <button class="btn btn-sm btn-danger" onclick="restartDevice('soft')">🔄 Soft Reboot</button>
            <button class="btn btn-sm btn-danger" onclick="restartDevice('erase_nvs')">⚠️ Erase NVS</button>
          </div>
        </div>

        <!-- Tab: OTA Update -->
        <div id="tab-ota" style="display:none;flex-direction:column;gap:0.75rem;">
          <div style="font-size:0.85rem;">Upload new firmware binary (<code>.bin</code>)</div>
          <input type="file" id="ota-file" accept=".bin" style="background:rgba(0,0,0,0.3);padding:0.5rem;border-radius:0.5rem;border:1px solid var(--card-border);">
          <div class="fm-progress-bar"><div class="fm-progress-fill" id="ota-progress"></div></div>
          <button class="btn btn-full btn-success" onclick="uploadOTA()">⬆️ Flash Firmware Now</button>
        </div>
      </div>
    </div>
  </div>

  <div class="toast-container" id="toast-container"></div>

  <script>
    let currentFmPath = '/';
    let currentFiles = [];
    let selectedFiles = new Set();
    let viewMode = 'grid'; // 'grid' or 'list'

    // Format Uptime (sec/min/hours/day)
    function formatUptime(seconds) {
      const d = Math.floor(seconds / 86400);
      const h = Math.floor((seconds % 86400) / 3600);
      const m = Math.floor((seconds % 3600) / 60);
      const s = seconds % 60;
      if (d > 0) return `${d}d ${h.toString().padStart(2,'0')}h ${m.toString().padStart(2,'0')}m ${s.toString().padStart(2,'0')}s`;
      if (h > 0) return `${h}h ${m.toString().padStart(2,'0')}m ${s.toString().padStart(2,'0')}s`;
      if (m > 0) return `${m}m ${s.toString().padStart(2,'0')}s`;
      return `${s}s`;
    }

    // Init Stream
    window.addEventListener('DOMContentLoaded', () => {
      const port = (location.port === '' || location.port === '80') ? ':81' : ':81';
      document.getElementById('stream-img').src = `${location.protocol}//${location.hostname}${port}/stream`;
      pollTelemetry();
      setInterval(pollTelemetry, 2000);
      loadSystemSettings();
    });

    function showToast(msg) {
      const c = document.getElementById('toast-container');
      const t = document.createElement('div');
      t.className = 'toast';
      t.innerText = msg;
      c.appendChild(t);
      setTimeout(() => t.remove(), 3500);
    }

    function toggleNavDrawer() {
      document.getElementById('nav-drawer').classList.toggle('open');
      document.getElementById('nav-backdrop').classList.toggle('open');
    }
    function closeNavDrawer() {
      document.getElementById('nav-drawer').classList.remove('open');
      document.getElementById('nav-backdrop').classList.remove('open');
    }
    function toggleCamDrawer() {
      document.getElementById('cam-drawer').classList.toggle('open');
    }
    function openModal(id) { document.getElementById(id).classList.add('open'); }
    function closeModal(id) { document.getElementById(id).classList.remove('open'); }

    function openTelegramManager() {
      openModal('modal-settings');
      switchTab('tab-tg');
    }

    function switchTab(tabId) {
      ['tab-tg', 'tab-ntp', 'tab-sys', 'tab-ota'].forEach(t => {
        const el = document.getElementById(t);
        if (el) el.style.display = (t === tabId) ? 'flex' : 'none';
        const btn = document.getElementById('tab-btn-' + t.substring(4));
        if (btn) btn.style.background = (t === tabId) ? 'var(--accent)' : '#334155';
      });
    }

    function toggleViewMode() {
      viewMode = (viewMode === 'grid') ? 'list' : 'grid';
      document.getElementById('btn-view-mode').innerText = (viewMode === 'grid') ? '⊞' : '☰';
      renderFileList();
    }

    function updateControl(varName, val) {
      fetch(`/control?var=${varName}&val=${val}`).catch(() => {});
    }

    function capturePhoto() {
      showToast('📸 Capturing snapshot...');
      window.open('/capture', '_blank');
    }

    let flashState = 0;
    function toggleFlash() {
      flashState = flashState ? 0 : 1;
      fetch(`/api/system/flash?state=${flashState}`)
        .then(r => r.text())
        .then(st => {
          document.getElementById('btn-flash').style.background = (st === '1') ? 'var(--warning)' : 'var(--card-bg)';
          showToast(`💡 Flash is ${st === '1' ? 'ON' : 'OFF'}`);
        });
    }

    function saveCameraDefaults() {
      fetch('/api/camera/save', { method: 'POST' })
        .then(r => r.json())
        .then(d => {
          if (d.ok) showToast('💾 Camera defaults saved to flash!');
          else showToast('❌ Failed to save defaults');
        });
    }

    function testRawHTTPS() {
      showToast('🔒 Testing TLS handshake to Telegram...');
      const diag = document.getElementById('tg-diag-box');
      diag.style.display = 'block';
      diag.innerText = 'Connecting to api.telegram.org:443 via TLS...\nTesting handshake latency & certificate validation...';

      fetch('/api/telegram/test_https')
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            diag.style.color = '#38bdf8';
            diag.innerText = `✅ TLS Handshake SUCCESS (${d.tls_ms}ms)\n`
                           + `🌐 Method: ${d.method}\n`
                           + `🕒 ESP32 Clock: ${d.time}\n`
                           + `📡 Status: ${d.status}\n`
                           + `🤖 Response: ${d.resp}`;
            showToast(`✅ TLS Handshake OK (${d.tls_ms}ms)`);
          } else {
            diag.style.color = '#ef4444';
            diag.innerText = `❌ TLS Handshake FAILED\nError: ${d.err}\nTime: ${d.time || '--'}`;
            showToast(`❌ TLS Handshake Failed: ${d.err}`);
          }
        })
        .catch(err => {
          diag.style.color = '#ef4444';
          diag.innerText = '❌ Network request error while running TLS diagnostic';
          showToast('❌ Network error testing HTTPS');
        });
    }

    function sendTelegramTest(type) {
      showToast(`🤖 Sending Telegram test ${type}...`);
      fetch(`/api/telegram/test_${type}`, { method: 'POST' })
        .then(r => r.json())
        .then(d => {
          if (d.ok) showToast(`✅ Telegram test ${type} queued!`);
          else showToast(`❌ Telegram test failed`);
        });
    }

    function pollTelemetry() {
      fetch('/api/telemetry')
        .then(r => r.json())
        .then(d => {
          document.getElementById('live-indicator').classList.add('active');
          document.getElementById('bar-live-dot').classList.add('active');
          
          // WiFi RSSI & Uptime
          const rssiText = `📶 ${d.rssi} dBm`;
          const uptimeStr = formatUptime(d.uptime);
          
          document.getElementById('overlay-rssi').innerText = rssiText;
          document.getElementById('bar-rssi').innerText = `📶 WiFi: ${d.rssi} dBm`;
          document.getElementById('bar-uptime').innerText = `⏱ Uptime: ${uptimeStr}`;
          document.getElementById('bar-ip').innerText = `🌐 ${d.ip}`;
          if (d.time) {
            document.getElementById('bar-time').innerText = `🕒 ${d.time}`;
          }
          
          if (d.fps) {
            document.getElementById('overlay-fps').innerText = `${d.fps} FPS`;
            document.getElementById('bar-fps').innerText = `⚡ ${d.fps} FPS`;
          }
        })
        .catch(() => {
          document.getElementById('live-indicator').classList.remove('active');
          document.getElementById('bar-live-dot').classList.remove('active');
          document.getElementById('bar-status').innerText = 'Reconnecting...';
        });
    }

    // ─── File Manager Functions ──────────────────────────────────
    function openFileManager(path) {
      currentFmPath = path || '/';
      openModal('modal-fm');
      selectedFiles.clear();
      updateBatchBar();
      loadStorageInfo();
      loadDirectory(currentFmPath);
    }

    function loadStorageInfo() {
      fetch('/api/sdcard/info')
        .then(r => r.json())
        .then(d => {
          if (d.mounted) {
            const usedMB = (d.used / 1024).toFixed(1);
            const totMB = (d.total / 1024).toFixed(1);
            const pct = Math.round((d.used / d.total) * 100) || 0;
            document.getElementById('fm-storage-text').innerText = `${usedMB} MB / ${totMB} MB (${pct}%)`;
            document.getElementById('fm-progress-fill').style.width = `${pct}%`;
          } else {
            document.getElementById('fm-storage-text').innerText = 'No SD Card Mounted';
          }
        });
    }

    function loadDirectory(path) {
      currentFmPath = path || '/';
      updateBreadcrumbs(currentFmPath);
      const container = document.getElementById('fm-container');
      container.innerHTML = '<div style="text-align:center;padding:2rem;color:var(--text-muted);">Loading files...</div>';

      fetch(`/api/sdcard/list?path=${encodeURIComponent(currentFmPath)}`)
        .then(r => r.json())
        .then(d => {
          currentFmPath = d.path || currentFmPath;
          updateBreadcrumbs(currentFmPath);
          currentFiles = d.files || [];
          renderFileList();
        })
        .catch(() => {
          container.innerHTML = '<div style="text-align:center;padding:2rem;color:var(--danger);">Failed to read SD card</div>';
        });
    }

    function updateBreadcrumbs(path) {
      const bc = document.getElementById('fm-breadcrumbs');
      bc.innerHTML = '<span class="fm-crumb" onclick="loadDirectory(\'/\')">📁 Root</span>';
      if (path === '/' || !path) return;
      const parts = path.split('/').filter(p => p.length > 0);
      let cur = '';
      parts.forEach((p, idx) => {
        cur += '/' + p;
        const thisPath = cur;
        bc.innerHTML += ` <span style="color:var(--text-muted);">></span> <span class="fm-crumb" onclick="loadDirectory('${thisPath}')">${p}</span>`;
      });
    }

    function navigateUpDir() {
      if (currentFmPath === '/' || !currentFmPath) return;
      const idx = currentFmPath.lastIndexOf('/');
      const parent = (idx <= 0) ? '/' : currentFmPath.substring(0, idx);
      loadDirectory(parent);
    }

    function renderFileList() {
      const container = document.getElementById('fm-container');
      if (currentFiles.length === 0) {
        container.innerHTML = '<div style="text-align:center;padding:2.5rem;color:var(--text-muted);">📁 Folder is empty</div>';
        return;
      }

      if (viewMode === 'grid') {
        let html = '<div class="fm-grid">';
        currentFiles.forEach(f => {
          const isSelected = selectedFiles.has(f.path);
          const isDir = f.is_dir;
          const isImg = f.name.toLowerCase().endsWith('.jpg') || f.name.toLowerCase().endsWith('.jpeg');
          const isVid = f.name.toLowerCase().endsWith('.avi');
          const szStr = isDir ? 'Folder' : (f.size > 1048576) ? (f.size/1048576).toFixed(1)+'MB' : (f.size/1024).toFixed(0)+'KB';

          html += `
            <div class="fm-card ${isSelected ? 'selected' : ''}" onclick="handleItemClick('${f.path}', ${isDir}, event)">
              <input type="checkbox" class="fm-card-chk" ${isSelected ? 'checked' : ''} onclick="toggleSelect('${f.path}', event)">
              <div class="fm-card-preview">
                ${isDir ? '<span class="fm-card-icon">📁</span>' :
                  isImg ? `<img src="/api/sdcard/download?name=${encodeURIComponent(f.path)}&inline=1" loading="lazy">` :
                  isVid ? '<span class="fm-card-icon">🎬</span>' : '<span class="fm-card-icon">📄</span>'}
              </div>
              <div class="fm-card-title" title="${f.name}">${f.name}</div>
              <div class="fm-card-meta">${szStr}</div>
              <div class="fm-card-actions" onclick="event.stopPropagation()">
                ${!isDir ? `<a class="btn btn-sm btn-icon" href="/api/sdcard/download?name=${encodeURIComponent(f.path)}" title="Download" style="width:28px;height:28px;font-size:0.8rem;">⬇️</a>` : ''}
                <button class="btn btn-sm btn-icon btn-danger" onclick="deleteSingleFile('${f.path}')" title="Delete" style="width:28px;height:28px;font-size:0.8rem;">🗑️</button>
              </div>
            </div>`;
        });
        html += '</div>';
        container.innerHTML = html;
      } else {
        let html = '<div class="fm-list">';
        currentFiles.forEach(f => {
          const isSelected = selectedFiles.has(f.path);
          const isDir = f.is_dir;
          const szStr = isDir ? 'Folder' : (f.size > 1048576) ? (f.size/1048576).toFixed(1)+'MB' : (f.size/1024).toFixed(0)+'KB';
          html += `
            <div class="fm-row ${isSelected ? 'selected' : ''}" onclick="handleItemClick('${f.path}', ${isDir}, event)">
              <input type="checkbox" ${isSelected ? 'checked' : ''} onclick="toggleSelect('${f.path}', event)">
              <span style="font-size:1.2rem;">${isDir ? '📁' : f.name.endsWith('.avi') ? '🎬' : '📸'}</span>
              <div class="fm-row-name">${f.name}</div>
              <div class="fm-row-size">${szStr}</div>
              <div style="display:flex;gap:0.3rem;" onclick="event.stopPropagation()">
                ${!isDir ? `<a class="btn btn-sm btn-icon" href="/api/sdcard/download?name=${encodeURIComponent(f.path)}" title="Download" style="width:28px;height:28px;font-size:0.8rem;">⬇️</a>` : ''}
                <button class="btn btn-sm btn-icon btn-danger" onclick="deleteSingleFile('${f.path}')" title="Delete" style="width:28px;height:28px;font-size:0.8rem;">🗑️</button>
              </div>
            </div>`;
        });
        html += '</div>';
        container.innerHTML = html;
      }
    }

    function handleItemClick(path, isDir, ev) {
      if (isDir) {
        loadDirectory(path);
      } else {
        toggleSelect(path, ev);
      }
    }

    function toggleSelect(path, ev) {
      if (ev) ev.stopPropagation();
      if (selectedFiles.has(path)) selectedFiles.delete(path);
      else selectedFiles.add(path);
      updateBatchBar();
      renderFileList();
    }

    function selectAllFiles() {
      if (selectedFiles.size === currentFiles.length) {
        selectedFiles.clear();
      } else {
        currentFiles.forEach(f => selectedFiles.add(f.path));
      }
      updateBatchBar();
      renderFileList();
    }

    function clearSelection() {
      selectedFiles.clear();
      updateBatchBar();
      renderFileList();
    }

    function updateBatchBar() {
      const bar = document.getElementById('batch-bar');
      const count = document.getElementById('batch-count');
      if (selectedFiles.size > 0) {
        bar.classList.add('active');
        count.innerText = `${selectedFiles.size} items selected`;
      } else {
        bar.classList.remove('active');
      }
    }

    function deleteSingleFile(path) {
      if (!confirm(`Delete ${path}?`)) return;
      fetch(`/api/sdcard/delete?name=${encodeURIComponent(path)}`)
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            showToast('🗑️ File deleted');
            loadDirectory(currentFmPath);
            loadStorageInfo();
          } else {
            showToast('❌ Delete failed');
          }
        });
    }

    function deleteSelectedFiles() {
      if (selectedFiles.size === 0) return;
      if (!confirm(`Delete all ${selectedFiles.size} selected items?`)) return;
      const names = Array.from(selectedFiles).join(',');
      fetch(`/api/sdcard/delete?name=${encodeURIComponent(names)}`)
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            showToast(`🗑️ ${selectedFiles.size} items deleted`);
            selectedFiles.clear();
            updateBatchBar();
            loadDirectory(currentFmPath);
            loadStorageInfo();
          } else {
            showToast('❌ Batch delete failed');
          }
        });
    }

    function formatSDCard() {
      if (!confirm('⚠️ WARNING: This will permanently erase ALL files on the SD card!\n\nAre you sure you want to format?')) return;
      showToast('🧹 Formatting SD card...');
      fetch('/api/sdcard/format')
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            showToast('✅ SD card formatted successfully!');
            loadDirectory('/');
            loadStorageInfo();
          } else {
            showToast('❌ SD format failed');
          }
        });
    }

    // ─── Settings & OTA ──────────────────────────────────────────
    function loadSystemSettings() {
      fetch('/api/system')
        .then(r => r.json())
        .then(d => {
          document.getElementById('cfg-mdns').value = d.mdns || '';
          document.getElementById('cfg-ssid').value = d.ssid || '';
          document.getElementById('cfg-tg-token').value = d.tg_token || '';
          document.getElementById('cfg-tg-chat').value = d.tg_chat_id || '';
          if (d.ntp_server1) document.getElementById('cfg-ntp1').value = d.ntp_server1;
          if (d.ntp_server2) document.getElementById('cfg-ntp2').value = d.ntp_server2;
          if (d.ntp_offset !== undefined) document.getElementById('cfg-ntp-offset').value = d.ntp_offset;
          if (d.ntp_dst !== undefined) document.getElementById('chk-dst').checked = (d.ntp_dst === 1);
          if (d.system_time) document.getElementById('cfg-clock-display').innerText = d.system_time;

          if (d.fps) {
            document.getElementById('rng-fps').value = d.fps;
            document.getElementById('val-fps').innerText = d.fps + ' FPS';
            document.getElementById('overlay-fps').innerText = d.fps + ' FPS';
            document.getElementById('bar-fps').innerText = '⚡ ' + d.fps + ' FPS';
          }
        });
    }

    function saveSettings() {
      const params = new URLSearchParams({
        mdns_name: document.getElementById('cfg-mdns').value,
        wifi_ssid: document.getElementById('cfg-ssid').value,
        wifi_pass: document.getElementById('cfg-pass').value,
        tg_token:  document.getElementById('cfg-tg-token').value,
        tg_chat_id:document.getElementById('cfg-tg-chat').value,
        ntp_server1:document.getElementById('cfg-ntp1').value,
        ntp_server2:document.getElementById('cfg-ntp2').value,
        ntp_offset: document.getElementById('cfg-ntp-offset').value,
        ntp_dst:    document.getElementById('chk-dst').checked ? '1' : '0'
      });
      fetch('/api/system/config', { method: 'POST', body: params.toString() })
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            showToast('💾 Settings saved successfully!');
            setTimeout(loadSystemSettings, 800);
          } else showToast('❌ Failed to save settings');
        });
    }

    function restartDevice(type) {
      if (!confirm(`Are you sure you want to restart (${type})?`)) return;
      fetch(`/api/system/restart?type=${type}`)
        .then(() => showToast('🔄 Rebooting ESP32-CAM...'));
    }

    function uploadOTA() {
      const fileInput = document.getElementById('ota-file');
      if (!fileInput.files.length) { alert('Please select a .bin file'); return; }
      const file = fileInput.files[0];
      const xhr = new XMLHttpRequest();
      xhr.open('POST', '/ota', true);
      xhr.upload.onprogress = (e) => {
        if (e.lengthComputable) {
          const pct = Math.round((e.loaded / e.total) * 100);
          document.getElementById('ota-progress').style.width = pct + '%';
        }
      };
      xhr.onload = () => {
        if (xhr.status === 200) {
          showToast('✅ OTA update complete! Rebooting...');
          setTimeout(() => location.reload(), 8000);
        } else {
          showToast('❌ OTA update failed');
        }
      };
      xhr.send(file);
    }
  </script>
</body>
</html>
)rawliteral";
