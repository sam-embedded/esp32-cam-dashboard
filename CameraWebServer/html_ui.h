#pragma once
#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
  <meta name="theme-color" content="#0b0f19">
  <title>ESP32-CAM XiaoZhi AI Pro</title>
  <style>
    /* Modern Glassmorphic Dark Slate Theme */
    :root {
      --bg: #070a12;
      --card-bg: rgba(17, 24, 39, 0.85);
      --card-border: rgba(255, 255, 255, 0.08);
      --card-glow: rgba(56, 189, 248, 0.15);
      --bar-bg: linear-gradient(180deg, #162032 0%, #0d1524 100%);
      --bar-border: rgba(255, 255, 255, 0.1);
      --accent: #0284c7;
      --accent-hover: #0369a1;
      --accent-glow: rgba(2, 132, 199, 0.4);
      --accent-light: #38bdf8;
      --text: #f8fafc;
      --text-muted: #94a3b8;
      --success: #10b981;
      --danger: #ef4444;
      --warning: #f59e0b;
      --radius: 12px;
      --header-h: 48px;
      --nav-h: 56px;
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      -webkit-tap-highlight-color: transparent;
      user-select: none;
    }

    body {
      background: var(--bg);
      color: var(--text);
      min-height: 100dvh;
      display: flex;
      flex-direction: column;
      overflow-x: hidden;
    }

    /* ─── Top App Header ─── */
    .app-header {
      background: var(--bar-bg);
      border-bottom: 1px solid var(--bar-border);
      height: var(--header-h);
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 0 0.85rem;
      position: sticky;
      top: 0;
      z-index: 100;
      backdrop-filter: blur(12px);
      -webkit-backdrop-filter: blur(12px);
      box-shadow: 0 4px 12px rgba(0,0,0,0.5);
    }
    .brand-title {
      font-size: 1rem;
      font-weight: 700;
      color: #fff;
      display: flex;
      align-items: center;
      gap: 0.45rem;
      letter-spacing: -0.02em;
    }
    .brand-badge {
      font-size: 0.65rem;
      background: linear-gradient(135deg, #0284c7, #38bdf8);
      color: #fff;
      padding: 0.15rem 0.45rem;
      border-radius: 12px;
      font-weight: 700;
      text-transform: uppercase;
    }
    .header-actions {
      display: flex;
      align-items: center;
      gap: 0.4rem;
    }

    /* ─── Buttons ─── */
    .btn {
      background: linear-gradient(180deg, #243247 0%, #172233 100%);
      border: 1px solid rgba(255,255,255,0.12);
      color: var(--text);
      padding: 0.45rem 0.8rem;
      border-radius: 8px;
      font-size: 0.82rem;
      font-weight: 600;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 0.35rem;
      transition: all 0.15s ease;
      touch-action: manipulation;
      box-shadow: 0 2px 4px rgba(0,0,0,0.3);
    }
    .btn:active {
      transform: translateY(1px);
      box-shadow: inset 0 2px 4px rgba(0,0,0,0.5);
      background: #111a29;
    }
    .btn-icon {
      width: 36px;
      height: 36px;
      padding: 0;
      font-size: 1rem;
      border-radius: 8px;
    }
    .btn-accent {
      background: linear-gradient(180deg, #0284c7 0%, #0369a1 100%);
      border-color: #38bdf8;
      color: #fff;
      box-shadow: 0 0 10px var(--accent-glow);
    }
    .btn-accent:active { background: #0284c7; }
    .btn-success {
      background: linear-gradient(180deg, #10b981 0%, #059669 100%);
      border-color: #34d399;
      color: #fff;
    }
    .btn-danger {
      background: linear-gradient(180deg, #ef4444 0%, #dc2626 100%);
      border-color: #f87171;
      color: #fff;
    }
    .btn-block { width: 100%; }

    /* ─── Video Viewport (FROZEN / STICKY WHILE SCROLLING) ─── */
    .viewport-box {
      position: sticky;
      top: var(--header-h);
      z-index: 70;
      width: 100%;
      background: #000;
      height: clamp(190px, 32vh, 290px);
      display: flex;
      align-items: center;
      justify-content: center;
      overflow: hidden;
      border-bottom: 1px solid var(--bar-border);
      box-shadow: 0 4px 16px rgba(0,0,0,0.6);
    }
    #stream-img {
      max-width: 100%;
      max-height: 100%;
      object-fit: contain;
      display: block;
    }

    /* HUD Overlay on Video */
    .hud-overlay {
      position: absolute;
      top: 8px;
      left: 8px;
      background: rgba(11, 15, 25, 0.85);
      backdrop-filter: blur(8px);
      -webkit-backdrop-filter: blur(8px);
      border: 1px solid rgba(255,255,255,0.12);
      border-radius: 8px;
      padding: 0.3rem 0.6rem;
      font-size: 0.74rem;
      display: flex;
      align-items: center;
      gap: 0.45rem;
      pointer-events: none;
      z-index: 10;
      box-shadow: 0 4px 12px rgba(0,0,0,0.5);
    }
    .live-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: var(--danger);
      transition: all 0.3s;
    }
    .live-dot.active {
      background: var(--success);
      box-shadow: 0 0 8px var(--success);
    }

    /* ─── Action Toolbar ─── */
    .action-toolbar {
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 0.55rem 0.85rem;
      background: var(--bar-bg);
      border-bottom: 1px solid var(--bar-border);
      gap: 0.4rem;
      flex-wrap: wrap;
    }
    .tools-left, .tools-right {
      display: flex;
      align-items: center;
      gap: 0.4rem;
    }

    /* ─── Telemetry Strip ─── */
    .telemetry-strip {
      display: flex;
      flex-wrap: wrap;
      gap: 0.35rem;
      padding: 0.5rem 0.85rem;
      background: rgba(17, 24, 39, 0.95);
      border-bottom: 1px solid var(--card-border);
    }
    .pill {
      background: rgba(255,255,255,0.05);
      border: 1px solid rgba(255,255,255,0.08);
      border-radius: 6px;
      padding: 0.22rem 0.5rem;
      font-size: 0.74rem;
      font-weight: 500;
      display: inline-flex;
      align-items: center;
      gap: 0.3rem;
      white-space: nowrap;
    }

    /* ─── Scrollable Content & Panels ─── */
    .content-area {
      flex: 1;
      padding-bottom: calc(var(--nav-h) + env(safe-area-inset-bottom, 0px) + 1.5rem);
    }
    .section-pane {
      display: none;
      animation: fadeIn 0.2s ease-out;
    }
    .section-pane.active { display: block; }
    @keyframes fadeIn { from { opacity: 0; transform: translateY(6px); } to { opacity: 1; transform: translateY(0); } }

    /* ─── Card Containers & Lists ─── */
    .card {
      margin: 0.85rem;
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: var(--radius);
      box-shadow: 0 4px 16px rgba(0,0,0,0.3);
      overflow: hidden;
      backdrop-filter: blur(12px);
      -webkit-backdrop-filter: blur(12px);
    }
    .card-header {
      background: linear-gradient(180deg, #1e293b 0%, #152033 100%);
      border-bottom: 1px solid var(--card-border);
      padding: 0.65rem 0.95rem;
      font-size: 0.78rem;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.05em;
      color: var(--accent-light);
      display: flex;
      align-items: center;
      justify-content: space-between;
    }
    .form-group {
      padding: 0.75rem 0.95rem;
      border-bottom: 1px solid rgba(255,255,255,0.04);
      display: flex;
      flex-direction: column;
      gap: 0.4rem;
    }
    .form-group:last-child { border-bottom: none; }
    .form-row {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 0.5rem;
    }
    .form-label {
      font-size: 0.84rem;
      font-weight: 600;
      display: flex;
      align-items: center;
      gap: 0.35rem;
    }
    .form-hint {
      font-size: 0.72rem;
      color: var(--text-muted);
    }
    .val-badge {
      font-size: 0.78rem;
      font-weight: 700;
      color: var(--accent-light);
      background: rgba(56, 189, 248, 0.12);
      padding: 0.15rem 0.45rem;
      border-radius: 4px;
      border: 1px solid rgba(56, 189, 248, 0.25);
    }

    /* ─── Sliders & Controls ─── */
    input[type=range] {
      width: 100%;
      height: 7px;
      border-radius: 4px;
      background: rgba(255,255,255,0.12);
      outline: none;
      -webkit-appearance: none;
      cursor: pointer;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 22px;
      height: 22px;
      border-radius: 50%;
      background: linear-gradient(180deg, #38bdf8 0%, #0284c7 100%);
      border: 2px solid #fff;
      box-shadow: 0 2px 6px rgba(0,0,0,0.5);
      cursor: pointer;
    }
    select, input[type=text], input[type=password], input[type=number], input[type=file] {
      width: 100%;
      background: rgba(0,0,0,0.4);
      border: 1px solid var(--card-border);
      border-radius: 8px;
      color: #fff;
      padding: 0.55rem 0.75rem;
      font-size: 0.85rem;
      outline: none;
      user-select: text;
    }
    select:focus, input:focus {
      border-color: var(--accent-light);
      box-shadow: 0 0 0 2px var(--accent-glow);
    }

    /* Tactile Flipswitch */
    .switch-toggle {
      display: inline-flex;
      position: relative;
      width: 54px;
      height: 28px;
      background: #1e293b;
      border: 1px solid rgba(255,255,255,0.15);
      border-radius: 14px;
      cursor: pointer;
      transition: all 0.2s ease;
      flex-shrink: 0;
    }
    .switch-slider {
      position: absolute;
      top: 2px;
      left: 2px;
      width: 22px;
      height: 22px;
      background: #fff;
      border-radius: 50%;
      transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
      box-shadow: 0 2px 4px rgba(0,0,0,0.4);
    }
    .switch-toggle.active {
      background: #0284c7;
      border-color: #38bdf8;
    }
    .switch-toggle.active .switch-slider {
      transform: translateX(26px);
    }

    /* ─── Presets Bar ─── */
    .presets-grid {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 0.4rem;
      padding: 0.75rem 0.95rem;
      border-bottom: 1px solid var(--card-border);
    }
    .preset-chip {
      background: rgba(255,255,255,0.04);
      border: 1px solid var(--card-border);
      padding: 0.4rem 0.2rem;
      border-radius: 8px;
      text-align: center;
      cursor: pointer;
      font-size: 0.72rem;
      font-weight: 600;
      transition: all 0.15s;
    }
    .preset-chip:hover, .preset-chip:active {
      background: rgba(2, 132, 199, 0.25);
      border-color: #38bdf8;
      color: #fff;
    }

    /* ─── SD Card File Manager ─── */
    .fm-toolbar {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 0.4rem;
      padding: 0.6rem 0.85rem;
      background: rgba(0,0,0,0.25);
      border-bottom: 1px solid var(--card-border);
      flex-wrap: wrap;
    }
    .fm-breadcrumbs {
      display: flex;
      align-items: center;
      gap: 0.3rem;
      font-size: 0.82rem;
      overflow-x: auto;
      white-space: nowrap;
      flex: 1;
    }
    .fm-crumb {
      color: #38bdf8;
      cursor: pointer;
      font-weight: 600;
    }
    .fm-crumb:hover { text-decoration: underline; }
    .progress-bar {
      height: 8px;
      background: rgba(255,255,255,0.1);
      border-radius: 4px;
      overflow: hidden;
      margin-top: 0.35rem;
    }
    .progress-fill {
      height: 100%;
      background: linear-gradient(90deg, #0284c7, #10b981);
      width: 0%;
      transition: width 0.3s ease;
    }
    .batch-bar {
      display: none;
      align-items: center;
      justify-content: space-between;
      padding: 0.5rem 0.85rem;
      background: rgba(2, 132, 199, 0.2);
      border-bottom: 1px solid rgba(56, 189, 248, 0.3);
    }
    .batch-bar.active { display: flex; }

    /* SD Grid / List Layout */
    .fm-grid {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(130px, 1fr));
      gap: 0.75rem;
      padding: 0.85rem;
    }
    .fm-card {
      background: rgba(255, 255, 255, 0.03);
      border: 1px solid var(--card-border);
      border-radius: 8px;
      padding: 0.55rem;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 0.35rem;
      position: relative;
      cursor: pointer;
      transition: all 0.15s ease;
    }
    .fm-card:hover {
      background: rgba(255, 255, 255, 0.08);
      border-color: rgba(56, 189, 248, 0.4);
    }
    .fm-card.selected {
      background: rgba(2, 132, 199, 0.25);
      border-color: #38bdf8;
    }
    .fm-card-chk {
      position: absolute;
      top: 6px;
      left: 6px;
      width: 18px;
      height: 18px;
      cursor: pointer;
      z-index: 5;
    }
    .fm-card-preview {
      width: 100%;
      height: 88px;
      border-radius: 6px;
      background: rgba(0,0,0,0.4);
      display: flex;
      align-items: center;
      justify-content: center;
      overflow: hidden;
      font-size: 2.2rem;
    }
    .fm-card-preview img {
      width: 100%;
      height: 100%;
      object-fit: cover;
    }
    .fm-card-title {
      font-size: 0.74rem;
      font-weight: 600;
      width: 100%;
      text-align: center;
      white-space: nowrap;
      overflow: hidden;
      text-overflow: ellipsis;
    }
    .fm-card-meta {
      font-size: 0.68rem;
      color: var(--text-muted);
    }
    .fm-card-actions {
      display: flex;
      gap: 0.35rem;
      margin-top: 0.15rem;
    }

    .fm-list {
      display: flex;
      flex-direction: column;
    }
    .fm-row {
      display: flex;
      align-items: center;
      gap: 0.65rem;
      padding: 0.65rem 0.85rem;
      border-bottom: 1px solid rgba(255,255,255,0.05);
      cursor: pointer;
      transition: background 0.15s;
    }
    .fm-row:hover { background: rgba(255,255,255,0.05); }
    .fm-row.selected { background: rgba(2, 132, 199, 0.2); }
    .fm-row-name { flex: 1; font-size: 0.82rem; font-weight: 500; word-break: break-all; }
    .fm-row-size { font-size: 0.74rem; color: var(--text-muted); width: 70px; text-align: right; }

    /* ─── XiaoZhi AI Chat Console ─── */
    .chat-box {
      background: #060a12;
      border: 1px solid rgba(56, 189, 248, 0.25);
      border-radius: 8px;
      padding: 0.75rem;
      min-height: 160px;
      max-height: 280px;
      overflow-y: auto;
      font-family: monospace;
      font-size: 0.78rem;
      display: flex;
      flex-direction: column;
      gap: 0.5rem;
    }
    .chat-bubble {
      padding: 0.5rem 0.75rem;
      border-radius: 8px;
      line-height: 1.4;
      white-space: pre-wrap;
      word-break: break-word;
    }
    .chat-bubble.ai {
      background: rgba(2, 132, 199, 0.2);
      border: 1px solid rgba(56, 189, 248, 0.3);
      color: #38bdf8;
      align-self: flex-start;
      max-width: 90%;
    }
    .chat-bubble.user {
      background: rgba(255, 255, 255, 0.08);
      border: 1px solid rgba(255, 255, 255, 0.12);
      color: #fff;
      align-self: flex-end;
      max-width: 85%;
    }
    .prompt-chips {
      display: flex;
      gap: 0.35rem;
      overflow-x: auto;
      padding: 0.4rem 0;
    }
    .chip {
      background: rgba(56, 189, 248, 0.1);
      border: 1px solid rgba(56, 189, 248, 0.25);
      color: #38bdf8;
      padding: 0.25rem 0.6rem;
      border-radius: 14px;
      font-size: 0.72rem;
      font-weight: 600;
      white-space: nowrap;
      cursor: pointer;
    }
    .chip:hover {
      background: rgba(56, 189, 248, 0.25);
    }
    .btn-mic {
      background: rgba(56, 189, 248, 0.15);
      border: 1px solid rgba(56, 189, 248, 0.4);
      color: #38bdf8;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      transition: all 0.2s ease;
      font-size: 1.1rem;
      padding: 0.4rem 0.65rem;
    }
    .btn-mic.listening {
      background: rgba(239, 68, 68, 0.25);
      border-color: #ef4444;
      color: #f87171;
      animation: pulseMic 1.2s infinite;
    }
    @keyframes pulseMic {
      0% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0.6); }
      70% { box-shadow: 0 0 0 8px rgba(239, 68, 68, 0); }
      100% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0); }
    }

    /* ─── Persistent Bottom Navigation Bar ─── */
    .bottom-nav {
      position: fixed;
      bottom: 0;
      left: 0;
      right: 0;
      height: calc(var(--nav-h) + env(safe-area-inset-bottom, 0px));
      padding-bottom: env(safe-area-inset-bottom, 0px);
      background: var(--bar-bg);
      border-top: 1px solid var(--bar-border);
      display: flex;
      z-index: 100;
      box-shadow: 0 -4px 16px rgba(0,0,0,0.6);
    }
    .nav-btn {
      flex: 1;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 2px;
      padding: 0.3rem 0.2rem;
      background: transparent;
      border: none;
      border-right: 1px solid rgba(255,255,255,0.06);
      color: var(--text-muted);
      font-size: 0.68rem;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.03em;
      cursor: pointer;
      transition: all 0.15s ease;
      touch-action: manipulation;
    }
    .nav-btn:last-child { border-right: none; }
    .nav-btn:active { background: rgba(2, 132, 199, 0.12); }
    .nav-btn.active {
      color: #38bdf8;
      background: rgba(2, 132, 199, 0.2);
      box-shadow: inset 0 3px 0 #38bdf8;
    }
    .nav-btn .nav-icon { font-size: 1.25rem; }

    /* ─── Lightbox Modal ─── */
    .modal-backdrop {
      position: fixed;
      inset: 0;
      background: rgba(0,0,0,0.85);
      backdrop-filter: blur(8px);
      -webkit-backdrop-filter: blur(8px);
      display: none;
      align-items: center;
      justify-content: center;
      z-index: 500;
      padding: 1rem;
    }
    .modal-backdrop.open { display: flex; }
    .modal-card {
      background: var(--bg);
      border: 1px solid var(--card-border);
      border-radius: 12px;
      width: 100%;
      max-width: 600px;
      overflow: hidden;
      box-shadow: 0 20px 40px rgba(0,0,0,0.8);
      animation: popIn 0.2s cubic-bezier(0.16, 1, 0.3, 1);
    }
    @keyframes popIn { from { transform: scale(0.92); opacity: 0; } to { transform: scale(1); opacity: 1; } }
    .modal-header {
      padding: 0.75rem 1rem;
      background: var(--bar-bg);
      border-bottom: 1px solid var(--bar-border);
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-weight: 700;
      font-size: 0.92rem;
    }
    .modal-body {
      padding: 1rem;
      display: flex;
      flex-direction: column;
      gap: 0.75rem;
      max-height: 80vh;
      overflow-y: auto;
    }

    /* ─── Toast Notifications ─── */
    .toast-box {
      position: fixed;
      bottom: calc(var(--nav-h) + env(safe-area-inset-bottom, 0px) + 12px);
      left: 50%;
      transform: translateX(-50%);
      z-index: 999;
      pointer-events: none;
      display: flex;
      flex-direction: column;
      gap: 0.5rem;
      align-items: center;
      width: 90%;
      max-width: 400px;
    }
    .toast-msg {
      background: rgba(15, 23, 42, 0.95);
      border: 1px solid #38bdf8;
      color: #fff;
      padding: 0.55rem 1rem;
      border-radius: 20px;
      font-size: 0.82rem;
      font-weight: 600;
      box-shadow: 0 8px 24px rgba(0,0,0,0.6);
      text-align: center;
      animation: toastAnim 0.25s ease-out;
      pointer-events: auto;
    }
    @keyframes toastAnim {
      from { opacity: 0; transform: translateY(12px); }
      to { opacity: 1; transform: translateY(0); }
    }

    /* ─── DESKTOP DASHBOARD (≥ 960px) ─── */
    @media (min-width: 960px) {
      body { height: 100vh; overflow: hidden; }
      .main-layout {
        display: grid;
        grid-template-columns: 1.15fr 1fr;
        height: calc(100vh - var(--header-h));
        overflow: hidden;
      }
      .desktop-left-pane {
        display: flex;
        flex-direction: column;
        border-right: 1px solid var(--bar-border);
        background: #000;
        height: 100%;
        overflow-y: auto;
      }
      .viewport-box {
        position: static;
        flex: 1;
        min-height: 320px;
        height: unset;
        max-height: unset;
      }
      .desktop-right-pane {
        display: flex;
        flex-direction: column;
        height: 100%;
        overflow-y: auto;
        background: var(--bg);
      }
      .content-area {
        padding-bottom: calc(var(--nav-h) + 1rem);
      }
    }
  </style>
</head>
<body>

  <!-- ─── Top App Header ─── -->
  <header class="app-header">
    <div class="brand-title">
      <span>📷</span> ESP32-CAM <span class="brand-badge">XiaoZhi AI</span>
    </div>
    <div class="header-actions">
      <button class="btn btn-icon" id="header-btn-flash" onclick="toggleFlash()" title="Toggle Flash Spotlight">💡</button>
      <button class="btn btn-icon btn-accent" onclick="capturePhoto()" title="Take Snapshot Photo">📷</button>
    </div>
  </header>

  <!-- ─── Main Unified Layout ─── -->
  <div class="main-layout">

    <!-- ─── Video & Stream Section ─── -->
    <div class="desktop-left-pane">
      <!-- Video Viewport (FROZEN / STICKY AT TOP WHILE SCROLLING) -->
      <div class="viewport-box" id="viewport-box">
        <div class="hud-overlay">
          <div class="live-dot" id="live-indicator"></div>
          <span id="hud-status" style="font-weight:700;">LIVE</span>
          <span style="color:var(--text-muted);">|</span>
          <span id="hud-fps">25 FPS</span>
          <span style="color:var(--text-muted);">|</span>
          <span id="hud-rssi">📶 -- dBm</span>
        </div>
        <img id="stream-img" src="" alt="ESP32-CAM Stream">
      </div>

      <!-- Quick Action Toolbar Under Video -->
      <div class="action-toolbar">
        <div class="tools-left">
          <button class="btn btn-accent" onclick="capturePhoto()">📷 Snapshot</button>
          <button class="btn" id="btn-flash" onclick="toggleFlash()">💡 Flash OFF</button>
          <button class="btn" onclick="startStream()">🔄 Reload</button>
        </div>
        <div class="tools-right">
          <button class="btn btn-icon" onclick="toggleFullscreen()" title="Fullscreen">⛶</button>
        </div>
      </div>

      <!-- Stream FPS & Hardware Telemetry Strip -->
      <div class="telemetry-strip">
        <div class="pill" style="color:#38bdf8;">
          <span class="live-dot" id="pill-live-dot"></span>
          <span id="stat-status">Connecting...</span>
        </div>
        <div class="pill" id="stat-fps">⚡ 25 FPS</div>
        <div class="pill" id="stat-rssi">📶 -- dBm</div>
        <div class="pill" id="stat-sd" style="color:#10b981;">💾 SD --</div>
        <div class="pill" id="stat-rec" style="display:none;color:#ef4444;font-weight:700;">🔴 REC</div>
        <div class="pill" id="stat-time" style="color:#38bdf8;">🕒 --</div>
        <div class="pill" id="stat-heap" style="color:var(--text-muted);">🧠 --</div>
        <div class="pill" id="stat-uptime" style="color:#10b981;">⏱ 0s</div>
        <div class="pill" id="stat-ip" style="color:var(--text-muted);">🌐 --</div>
      </div>
    </div>

    <!-- ─── Control Center & Active Sections ─── -->
    <div class="desktop-right-pane content-area">

      <!-- ─── 1. CAMERA SENSOR CONTROL CENTER (FULL CONTROL) ─── -->
      <section class="section-pane active" id="pane-cam">
        <div class="card">
          <div class="card-header">
            <span>⚡ Quick Camera Presets</span>
          </div>
          <div class="presets-grid">
            <div class="preset-chip" onclick="applyPreset('turbo')">⚡ Turbo 25fps</div>
            <div class="preset-chip" onclick="applyPreset('night')">🌙 Night Vision</div>
            <div class="preset-chip" onclick="applyPreset('daylight')">☀️ Daylight Pro</div>
            <div class="preset-chip" onclick="applyPreset('hd')">📸 Ultra 2MP</div>
          </div>
        </div>

        <div class="card">
          <div class="card-header">
            <span>Stream Resolution & Pacing</span>
            <span class="val-badge" id="val-fps-badge">25 FPS</span>
          </div>

          <div class="form-group">
            <span class="form-label">Frame Resolution</span>
            <select id="sel-res" onchange="updateControl('framesize', this.value)">
              <option value="10">UXGA (1600x1200) - 2MP Full HD</option>
              <option value="9">SXGA (1280x1024)</option>
              <option value="8">XGA (1024x768)</option>
              <option value="7">SVGA (800x600)</option>
              <option value="6" selected>VGA (640x480) - Real-time 25fps</option>
              <option value="5">CIF (400x296)</option>
              <option value="4">QVGA (320x240) - Fast low-bitrate</option>
            </select>
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Stream FPS Limit</span>
              <span class="val-badge" id="disp-fps">25</span>
            </div>
            <input type="range" min="1" max="30" value="25" id="rng-fps" oninput="document.getElementById('disp-fps').innerText=this.value" onchange="updateControl('fps', this.value); document.getElementById('val-fps-badge').innerText=this.value+' FPS';">
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">JPEG Quality (Lower = Higher Clarity)</span>
              <span class="val-badge" id="disp-quality">14</span>
            </div>
            <input type="range" min="10" max="63" value="14" id="rng-quality" oninput="document.getElementById('disp-quality').innerText=this.value" onchange="updateControl('quality', this.value)">
          </div>
        </div>

        <div class="card">
          <div class="card-header">Color & Exposure Tuning</div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Brightness</span>
              <span class="val-badge" id="disp-bright">0</span>
            </div>
            <input type="range" min="-2" max="2" value="0" id="rng-bright" oninput="document.getElementById('disp-bright').innerText=this.value" onchange="updateControl('brightness', this.value)">
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Contrast</span>
              <span class="val-badge" id="disp-contrast">0</span>
            </div>
            <input type="range" min="-2" max="2" value="0" id="rng-contrast" oninput="document.getElementById('disp-contrast').innerText=this.value" onchange="updateControl('contrast', this.value)">
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Saturation</span>
              <span class="val-badge" id="disp-sat">0</span>
            </div>
            <input type="range" min="-2" max="2" value="0" id="rng-sat" oninput="document.getElementById('disp-sat').innerText=this.value" onchange="updateControl('saturation', this.value)">
          </div>

          <div class="form-group">
            <span class="form-label">Special Effect</span>
            <select id="sel-effect" onchange="updateControl('special_effect', this.value)">
              <option value="0">No Effect (Natural)</option>
              <option value="1">Negative</option>
              <option value="2">Grayscale</option>
              <option value="3">Red Tint</option>
              <option value="4">Green Tint</option>
              <option value="5">Blue Tint</option>
              <option value="6">Sepia</option>
            </select>
          </div>

          <div class="form-group">
            <span class="form-label">White Balance Mode</span>
            <select id="sel-wb" onchange="updateControl('wb_mode', this.value)">
              <option value="0">Auto White Balance</option>
              <option value="1">Sunny (Outdoor)</option>
              <option value="2">Cloudy</option>
              <option value="3">Office (Fluorescent)</option>
              <option value="4">Home (Incandescent)</option>
            </select>
          </div>
        </div>

        <div class="card">
          <div class="card-header">Advanced Sensor & Exposure Controls</div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Auto Exposure Control (AEC)</span>
              <div class="switch-toggle active" id="sw-aec" onclick="toggleSwitch('sw-aec', 'aec')"><div class="switch-slider"></div></div>
            </div>
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">AEC2 Night/Day Exposure Mode</span>
              <div class="switch-toggle" id="sw-aec2" onclick="toggleSwitch('sw-aec2', 'aec2')"><div class="switch-slider"></div></div>
            </div>
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Auto Gain Control (AGC)</span>
              <div class="switch-toggle active" id="sw-agc" onclick="toggleSwitch('sw-agc', 'agc')"><div class="switch-slider"></div></div>
            </div>
          </div>

          <div class="form-group">
            <span class="form-label">Gain Ceiling (Max Sensitivity Boost)</span>
            <select id="sel-gainceiling" onchange="updateControl('gainceiling', this.value)">
              <option value="0">2x</option>
              <option value="1">4x</option>
              <option value="2">8x</option>
              <option value="3">16x</option>
              <option value="4">32x</option>
              <option value="5">64x</option>
              <option value="6">128x (Extreme Low-Light)</option>
            </select>
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Auto White Balance Gain (AWB Gain)</span>
              <div class="switch-toggle active" id="sw-awb-gain" onclick="toggleSwitch('sw-awb-gain', 'awb_gain')"><div class="switch-slider"></div></div>
            </div>
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Black Pixel Correction (BPC)</span>
              <div class="switch-toggle" id="sw-bpc" onclick="toggleSwitch('sw-bpc', 'bpc')"><div class="switch-slider"></div></div>
            </div>
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">White Pixel Correction (WPC)</span>
              <div class="switch-toggle active" id="sw-wpc" onclick="toggleSwitch('sw-wpc', 'wpc')"><div class="switch-slider"></div></div>
            </div>
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Lens Distortion Correction (LENC)</span>
              <div class="switch-toggle" id="sw-lenc" onclick="toggleSwitch('sw-lenc', 'lenc')"><div class="switch-slider"></div></div>
            </div>
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Vertical Flip</span>
              <div class="switch-toggle" id="sw-vflip" onclick="toggleSwitch('sw-vflip', 'vflip')"><div class="switch-slider"></div></div>
            </div>
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Horizontal Mirror</span>
              <div class="switch-toggle" id="sw-hmirror" onclick="toggleSwitch('sw-hmirror', 'hmirror')"><div class="switch-slider"></div></div>
            </div>
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Color Bar Test Pattern</span>
              <div class="switch-toggle" id="sw-colorbar" onclick="toggleSwitch('sw-colorbar', 'colorbar')"><div class="switch-slider"></div></div>
            </div>
          </div>

          <div class="form-group">
            <button class="btn btn-block btn-success" onclick="saveCameraDefaults()">💾 Save Stream Defaults to Flash</button>
          </div>
        </div>
      </section>

      <!-- ─── 2. SD CARD FILE MANAGER (RECURSIVE DELETION FIXED) ─── -->
      <section class="section-pane" id="pane-sd">
        <div class="card">
          <div class="card-header">
            <span>SD Card Storage</span>
            <span id="sd-usage-text" class="val-badge">Loading...</span>
          </div>
          <div class="form-group">
            <div class="progress-bar"><div class="progress-fill" id="sd-progress-fill"></div></div>
          </div>
        </div>

        <div class="card" style="display:flex;flex-direction:column;">
          <div class="fm-toolbar">
            <div class="fm-breadcrumbs" id="fm-breadcrumbs">
              <span class="fm-crumb" onclick="loadDirectory('/')">📁 Root</span>
            </div>
            <div style="display:flex;gap:0.35rem;">
              <button class="btn" id="btn-view-mode" onclick="toggleViewMode()" title="Toggle View">⊞ Grid</button>
              <button class="btn" onclick="navigateUpDir()" title="Parent Directory">⬆️ Up</button>
              <button class="btn" onclick="selectAllFiles()" title="Select All">☑️ All</button>
              <button class="btn" onclick="loadDirectory(currentFmPath)" title="Refresh">🔄</button>
            </div>
          </div>

          <!-- Batch Action Bar -->
          <div class="batch-bar" id="batch-bar">
            <span id="batch-count" style="font-size:0.82rem;font-weight:700;color:#38bdf8;">0 selected</span>
            <div style="display:flex;gap:0.35rem;">
              <button class="btn btn-danger" onclick="deleteSelectedFiles()">🗑️ Delete Selected</button>
              <button class="btn" onclick="clearSelection()">✕ Cancel</button>
            </div>
          </div>

          <!-- Files Grid / List -->
          <div id="fm-container" style="min-height:260px;overflow-y:auto;">
            <div style="text-align:center;padding:2.5rem;color:var(--text-muted);">Loading files...</div>
          </div>

          <div class="form-group" style="border-top:1px solid var(--card-border);display:flex;justify-content:space-between;align-items:center;">
            <span class="form-hint">Erase & format all files on SD card</span>
            <button class="btn btn-danger" onclick="formatSDCard()">🧹 Format SD Card</button>
          </div>
        </div>
      </section>

      <!-- ─── 3. XIAOZHI AI AGENT & TELEGRAM CONSOLE ─── -->
      <section class="section-pane" id="pane-ai">
        <!-- XiaoZhi Device Pairing Code Card -->
        <div class="card" style="background:linear-gradient(135deg, rgba(2,132,199,0.15), rgba(99,102,241,0.15));border-color:rgba(56,189,248,0.4);">
          <div class="card-header">
            <span>🔑 XiaoZhi Device Pairing Code</span>
            <span class="val-badge" id="xz-link-badge" style="background:rgba(234,179,8,0.2);color:#facc15;">Unlinked</span>
          </div>
          <div class="form-group" style="text-align:center;padding:0.75rem 0.5rem;">
            <div style="font-size:0.75rem;color:var(--text-muted);margin-bottom:0.25rem;">Announced on Start / Restart — Share to Telegram or xiaozhi.me</div>
            <div id="xz-code-display" style="font-size:2.2rem;font-weight:900;letter-spacing:0.35rem;color:#38bdf8;font-family:monospace;text-shadow:0 0 15px rgba(56,189,248,0.5);margin:0.25rem 0;">------</div>
            <div style="font-size:0.75rem;color:var(--text-muted);margin-bottom:0.75rem;">Reply <code style="color:#38bdf8;background:rgba(0,0,0,0.3);padding:2px 6px;border-radius:4px;">/bind <span id="xz-code-inline">------</span></code> in Telegram to link your account</div>
            <div style="display:grid;grid-template-columns:1fr 1fr 1fr;gap:0.4rem;">
              <button class="btn btn-accent" onclick="announcePairingCode()" title="Broadcast code to Telegram Bot via text and voice note">📢 Announce</button>
              <button class="btn" onclick="regenPairingCode()" title="Generate fresh 6-digit code">🔄 New Code</button>
              <a class="btn" href="https://xiaozhi.me" target="_blank" style="text-decoration:none;display:flex;align-items:center;justify-content:center;" title="Open xiaozhi.me console">🔗 xiaozhi.me</a>
            </div>
          </div>
        </div>

        <div class="card">
          <div class="card-header">
            <span>🤖 XiaoZhi AI (小智) Live Assistant</span>
            <span class="val-badge">Agent Active</span>
          </div>

          <div class="form-group">
            <div class="chat-box" id="ai-chat-box">
              <div class="chat-bubble ai">✨ Hello! I am XiaoZhi AI (小智), your edge AI agent running on ESP32-CAM.
I have full control over camera capture, flash spotlight, SD card storage, and system health. You can talk to me naturally right here or from Telegram!</div>
            </div>

            <!-- Quick Action Chips -->
            <div class="prompt-chips">
              <div class="chip" onclick="sendAiPrompt('code')">🔑 Pairing Code</div>
              <div class="chip" onclick="sendAiPrompt('take a photo')">📸 Take Photo</div>
              <div class="chip" onclick="sendAiPrompt('turn on flash')">💡 Flash On</div>
              <div class="chip" onclick="sendAiPrompt('flash off')">💡 Flash Off</div>
              <div class="chip" onclick="sendAiPrompt('status report')">📊 System Status</div>
              <div class="chip" onclick="sendAiPrompt('check sd storage')">💾 SD Card</div>
              <div class="chip" onclick="sendAiPrompt('start record')">🎬 Start Record</div>
              <div class="chip" onclick="sendAiPrompt('who are you')">ℹ️ Who Are You</div>
            </div>

            <div style="display:flex;gap:0.4rem;margin-top:0.4rem;">
              <input type="text" id="ai-input" placeholder="Ask XiaoZhi AI or speak..." onkeydown="if(event.key==='Enter') sendCustomPrompt()">
              <button class="btn btn-mic" id="btn-mic" onclick="toggleMic()" title="Voice Input (ASR)">🎙️</button>
              <button class="btn btn-accent" onclick="sendCustomPrompt()">Send</button>
            </div>

            <div style="display:flex;align-items:center;justify-content:space-between;background:rgba(255,255,255,0.03);padding:0.45rem 0.65rem;border-radius:6px;margin-top:0.5rem;font-size:0.8rem;border:1px solid rgba(255,255,255,0.05);">
              <span style="color:var(--text-muted);">🔊 Browser Voice Output (TTS)</span>
              <div class="switch-toggle active" id="sw-ai-voice" onclick="toggleVoiceOutput()"><div class="switch-slider"></div></div>
            </div>
          </div>
        </div>

        <!-- XiaoZhi Agent Personality & Model Settings -->
        <div class="card" style="border-color:rgba(99,102,241,0.4);background:linear-gradient(180deg, rgba(99,102,241,0.08), rgba(15,23,42,0.6));">
          <div class="card-header">
            <span>⚙️ XiaoZhi Agent Personality & Model Settings</span>
            <span class="val-badge" id="xz-model-badge" style="background:rgba(99,102,241,0.25);color:#a5b4fc;">Edge Engine</span>
          </div>

          <div class="form-group">
            <span class="form-label">Agent Name</span>
            <input type="text" id="cfg-xz-name" placeholder="XiaoZhi AI (小智)">
            <span class="form-hint">Display name used in Telegram messages & voice greetings</span>
          </div>

          <div class="form-group">
            <span class="form-label">Agent Role / Persona</span>
            <input type="text" id="cfg-xz-role" placeholder="Autonomous Vision Guardian & Assistant">
            <span class="form-hint">Character role (e.g. Smart Home Security, Private Butler, Vision Bot)</span>
          </div>

          <div class="form-group">
            <span class="form-label">System Prompt / Custom Instructions</span>
            <textarea id="cfg-xz-prompt" rows="3" style="width:100%;box-sizing:border-box;background:rgba(0,0,0,0.3);border:1px solid var(--input-border);border-radius:6px;color:var(--text);padding:0.5rem;font-family:inherit;font-size:0.8rem;resize:vertical;" placeholder="You are an autonomous AI camera guardian. Protect the premises and respond concisely."></textarea>
            <span class="form-hint">Direct instruction guidelines for the AI agent's behavior</span>
          </div>

          <div class="form-group">
            <div class="form-row">
              <div>
                <span class="form-label">AI Backend Provider</span>
                <select id="sel-xz-provider" onchange="onXzProviderChange()">
                  <option value="edge">⚡ Edge Autonomous Engine (Local Fast, 0-Latency)</option>
                  <option value="deepseek">🧠 DeepSeek (deepseek-chat)</option>
                  <option value="openai">🤖 OpenAI (gpt-4o-mini / gpt-4o)</option>
                  <option value="custom">🌐 Custom OpenAI-compatible API</option>
                </select>
              </div>
              <div>
                <span class="form-label">Speech Voice Language</span>
                <select id="sel-xz-lang">
                  <option value="auto">🌐 Auto-detect (Multilingual)</option>
                  <option value="en">🇺🇸 English (en-US)</option>
                  <option value="zh-CN">🇨🇳 Mandarin (zh-CN)</option>
                  <option value="es">🇪🇸 Spanish (es-ES)</option>
                  <option value="hi">🇮🇳 Hindi (hi-IN)</option>
                </select>
              </div>
            </div>
          </div>

          <div id="xz-cloud-group" style="display:none;">
            <div class="form-group">
              <span class="form-label">API Key</span>
              <input type="password" id="cfg-xz-key" placeholder="sk-...">
              <span class="form-hint">Your API secret key for DeepSeek, OpenAI, or custom LLM</span>
            </div>

            <div class="form-group">
              <div class="form-row">
                <div>
                  <span class="form-label">API Endpoint URL</span>
                  <input type="text" id="cfg-xz-url" placeholder="https://api.deepseek.com/chat/completions">
                </div>
                <div>
                  <span class="form-label">Model Name</span>
                  <input type="text" id="cfg-xz-model" placeholder="deepseek-chat">
                </div>
              </div>
            </div>
          </div>

          <div class="form-group">
            <span class="form-label">MCP Tool Actions Enabled</span>
            <div style="display:grid;grid-template-columns:1fr 1fr;gap:0.4rem;font-size:0.8rem;margin-top:0.3rem;">
              <label style="display:flex;align-items:center;gap:0.4rem;cursor:pointer;"><input type="checkbox" id="chk-tool-photo" checked> 📸 Camera Snapshot</label>
              <label style="display:flex;align-items:center;gap:0.4rem;cursor:pointer;"><input type="checkbox" id="chk-tool-flash" checked> 💡 Flash Spotlight</label>
              <label style="display:flex;align-items:center;gap:0.4rem;cursor:pointer;"><input type="checkbox" id="chk-tool-rec" checked> 🎬 SD Video Record</label>
              <label style="display:flex;align-items:center;gap:0.4rem;cursor:pointer;"><input type="checkbox" id="chk-tool-telemetry" checked> 📊 Health Telemetry</label>
            </div>
          </div>

          <div class="form-group" style="display:flex;justify-content:flex-end;">
            <button class="btn btn-accent" onclick="saveAgentSettings()">💾 Save Agent Settings</button>
          </div>
        </div>

        <div class="card">
          <div class="card-header">Telegram Bot & Voice Integration</div>
          
          <div class="form-group">
            <span class="form-label">Bot Token</span>
            <input type="password" id="cfg-tg-token" placeholder="8967102688:AAHEieQC2_ZHa9ci0DiPsc3O4uLclWdLJ-k">
            <span class="form-hint">Obtain from @BotFather on Telegram</span>
          </div>

          <div class="form-group">
            <span class="form-label">Authorized Chat IDs</span>
            <input type="text" id="cfg-tg-chat" placeholder="318862528, 987654321">
            <span class="form-hint">Comma-separated user or group chat IDs</span>
          </div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Telegram Voice Output (Spoken Audio Notes)</span>
              <div class="switch-toggle active" id="sw-tg-voice" onclick="toggleSwitch('sw-tg-voice', 'tg_voice')"><div class="switch-slider"></div></div>
            </div>
            <span class="form-hint">Bot sends voice audio notes alongside text replies & supports voice input</span>
          </div>

          <div class="form-group">
            <button class="btn btn-block btn-accent" onclick="saveSettings()">💾 Save Telegram Config</button>
          </div>
        </div>

        <div class="card">
          <div class="card-header">🧪 Live Diagnostics & Audio Tests</div>

          <div class="form-group">
            <div style="display:grid;grid-template-columns:1fr 1fr;gap:0.4rem;">
              <button class="btn btn-accent" onclick="testRawHTTPS()">🔒 TLS Handshake</button>
              <button class="btn" onclick="sendTelegramTest('msg')">✉️ Text Test</button>
            </div>
            <div style="display:grid;grid-template-columns:1fr 1fr;gap:0.4rem;margin-top:0.4rem;">
              <button class="btn" onclick="sendTelegramTest('photo')">📸 Photo Test</button>
              <button class="btn" onclick="sendTelegramTest('voice')">🔊 Voice Test</button>
            </div>

            <div id="tg-diag-box" style="display:none;margin-top:0.6rem;font-size:0.75rem;background:#060a12;border:1px solid rgba(56,189,248,0.25);padding:0.65rem;border-radius:6px;white-space:pre-wrap;font-family:monospace;color:#38bdf8;"></div>
          </div>
        </div>
      </section>

      <!-- ─── 4. SYSTEM & NETWORK SETTINGS ─── -->
      <section class="section-pane" id="pane-sys">
        <div class="card">
          <div class="card-header">24/7 SD Video Recording Engine</div>

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Enable Background Recording</span>
              <div class="switch-toggle active" id="sw-rec-enable" onclick="toggleSwitch('sw-rec-enable', 'rec_enabled')"><div class="switch-slider"></div></div>
            </div>
            <span class="form-hint">Captures continuous Motion-JPEG AVI video segments</span>
          </div>

          <div class="form-group">
            <span class="form-label">Segment Duration</span>
            <select id="sel-rec-interval" onchange="updateControl('rec_interval', this.value)">
              <option value="5">5 Minutes</option>
              <option value="10">10 Minutes</option>
              <option value="15" selected>15 Minutes (Recommended)</option>
              <option value="30">30 Minutes</option>
              <option value="60">60 Minutes</option>
            </select>
          </div>
        </div>

        <div class="card">
          <div class="card-header">Network & Hostname</div>
          
          <div class="form-group">
            <span class="form-label">mDNS Hostname</span>
            <input type="text" id="cfg-mdns" placeholder="esp32cam">
            <span class="form-hint">Access via http://esp32cam.local</span>
          </div>

          <div class="form-group">
            <span class="form-label">WiFi Network Name (SSID)</span>
            <input type="text" id="cfg-ssid" placeholder="FTTH">
          </div>

          <div class="form-group">
            <span class="form-label">WiFi Password</span>
            <input type="password" id="cfg-pass" placeholder="••••••••">
          </div>

          <div class="form-group">
            <button class="btn btn-block btn-accent" onclick="saveSettings()">💾 Save WiFi & Hostname</button>
          </div>
        </div>

        <div class="card">
          <div class="card-header">
            <span>NTP System Clock</span>
            <span class="val-badge" id="cfg-clock-display">--</span>
          </div>

          <div class="form-group">
            <span class="form-label">Timezone</span>
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

          <div class="form-group">
            <div class="form-row">
              <span class="form-label">Daylight Saving (+1h)</span>
              <div class="switch-toggle" id="sw-dst" onclick="toggleSwitch('sw-dst')"><div class="switch-slider"></div></div>
            </div>
          </div>

          <div class="form-group">
            <span class="form-label">Primary NTP Server</span>
            <input type="text" id="cfg-ntp1" placeholder="pool.ntp.org">
          </div>

          <div class="form-group">
            <button class="btn btn-block btn-success" onclick="saveSettings()">💾 Save & Sync Clock</button>
          </div>
        </div>

        <div class="card">
          <div class="card-header">Firmware OTA Flash</div>
          <div class="form-group">
            <span class="form-label">Select .bin Firmware File</span>
            <input type="file" id="ota-file" accept=".bin">
            <div class="progress-bar"><div class="progress-fill" id="ota-progress"></div></div>
            <button class="btn btn-block btn-accent" style="margin-top:0.5rem;" onclick="uploadOTA()">⬆️ Flash Firmware Now</button>
          </div>
        </div>

        <div class="card">
          <div class="card-header">Device Reboot & Recovery</div>
          <div class="form-group">
            <div style="display:grid;grid-template-columns:1fr 1fr;gap:0.4rem;">
              <button class="btn btn-danger" onclick="restartDevice('soft')">🔄 Soft Reboot</button>
              <button class="btn btn-danger" onclick="restartDevice('erase_nvs')">⚠️ Erase NVS</button>
            </div>
          </div>
        </div>
      </section>

    </div>

  </div>

  <!-- ─── Persistent Bottom Navigation Bar ─── -->
  <nav class="bottom-nav" id="stream-subnav">
    <button class="nav-btn active" onclick="switchSection('cam')" id="nav-btn-cam">
      <span class="nav-icon">🎛️</span>
      <span>Controls</span>
    </button>
    <button class="nav-btn" onclick="switchSection('sd')" id="nav-btn-sd">
      <span class="nav-icon">📁</span>
      <span>SD Card</span>
    </button>
    <button class="nav-btn" onclick="switchSection('ai')" id="nav-btn-ai">
      <span class="nav-icon">🤖</span>
      <span>XiaoZhi AI</span>
    </button>
    <button class="nav-btn" onclick="switchSection('sys')" id="nav-btn-sys">
      <span class="nav-icon">⚙️</span>
      <span>System</span>
    </button>
  </nav>

  <!-- ─── Lightbox Modal for Media Preview ─── -->
  <div class="modal-backdrop" id="modal-lightbox" onclick="closeLightbox()">
    <div class="modal-card" onclick="event.stopPropagation()">
      <div class="modal-header">
        <span id="lb-title" style="white-space:nowrap;overflow:hidden;text-overflow:ellipsis;max-width:320px;">Media Viewer</span>
        <button class="btn btn-icon" onclick="closeLightbox()" style="width:30px;height:30px;font-size:0.9rem;">✕</button>
      </div>
      <div class="modal-body" id="lb-body">
        <img id="lb-img" src="" alt="Snapshot" style="width:100%;border-radius:8px;object-fit:contain;max-height:55vh;">
        <div style="display:flex;justify-content:space-between;align-items:center;margin-top:0.4rem;">
          <span id="lb-meta" class="form-hint">Loading...</span>
          <div style="display:flex;gap:0.4rem;">
            <a class="btn btn-accent" id="lb-dl" href="#" download>⬇️ Download</a>
            <button class="btn btn-danger" id="lb-del" onclick="deleteLightboxFile()">🗑️ Delete</button>
          </div>
        </div>
      </div>
    </div>
  </div>

  <div class="toast-box" id="toast-box"></div>

  <!-- ─── Application JavaScript ─── -->
  <script>
    let currentFmPath = '/';
    let currentFiles = [];
    let selectedFiles = new Set();
    let viewMode = 'grid'; // 'grid' | 'list'
    let flashState = 0;
    let streamRetryTimer = null;
    let activeLightboxPath = '';
    let currentSection = 'cam';

    // ─── Format Uptime ──────────────────────────────────────────
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

    // ─── Section Switcher ───────────────────────────────────────
    function switchSection(secId) {
      currentSection = secId;

      ['cam', 'sd', 'ai', 'sys'].forEach(s => {
        const btn = document.getElementById('nav-btn-' + s);
        if (btn) btn.classList.toggle('active', s === secId);
        const pane = document.getElementById('pane-' + s);
        if (pane) pane.classList.toggle('active', s === secId);
      });

      if (secId === 'sd') {
        loadStorageInfo();
        loadDirectory(currentFmPath);
      }
    }

    // ─── Stream Control ─────────────────────────────────────────
    function startStream() {
      const img = document.getElementById('stream-img');
      const host = location.hostname;
      img.src = `${location.protocol}//${host}:81/stream?t=${Date.now()}`;
      document.getElementById('hud-status').innerText = 'LIVE';
      document.getElementById('stat-status').innerText = 'Streaming';
    }

    function onStreamError() {
      document.getElementById('live-indicator').classList.remove('active');
      document.getElementById('pill-live-dot').classList.remove('active');
      document.getElementById('hud-status').innerText = 'Reconnecting...';
      document.getElementById('stat-status').innerText = 'Reconnecting...';
      document.getElementById('stream-img').src = '';
      if (streamRetryTimer) clearTimeout(streamRetryTimer);
      streamRetryTimer = setTimeout(startStream, 3000);
    }

    function onStreamLoad() {
      document.getElementById('live-indicator').classList.add('active');
      document.getElementById('pill-live-dot').classList.add('active');
      document.getElementById('hud-status').innerText = 'LIVE';
      document.getElementById('stat-status').innerText = 'Streaming';
    }

    function toggleFullscreen() {
      const box = document.getElementById('viewport-box');
      if (!document.fullscreenElement) {
        if (box.requestFullscreen) box.requestFullscreen();
        else if (box.webkitRequestFullscreen) box.webkitRequestFullscreen();
      } else {
        if (document.exitFullscreen) document.exitFullscreen();
      }
    }

    // ─── Telemetry Polling ──────────────────────────────────────
    function pollTelemetry() {
      fetch('/api/telemetry')
        .then(r => r.json())
        .then(d => {
          document.getElementById('live-indicator').classList.add('active');
          document.getElementById('pill-live-dot').classList.add('active');
          document.getElementById('stat-status').innerText = 'Streaming';

          // RSSI
          const rssiText = `📶 ${d.rssi} dBm`;
          document.getElementById('hud-rssi').innerText = rssiText;
          document.getElementById('stat-rssi').innerText = rssiText;

          // Uptime
          const uptimeStr = formatUptime(d.uptime);
          document.getElementById('stat-uptime').innerText = `⏱ ${uptimeStr}`;

          // IP
          document.getElementById('stat-ip').innerText = `🌐 ${d.ip}`;

          // Clock
          if (d.time && d.time !== '--') {
            document.getElementById('stat-time').innerText = `🕒 ${d.time}`;
          }

          // FPS
          if (d.fps) {
            document.getElementById('hud-fps').innerText = `${d.fps} FPS`;
            document.getElementById('stat-fps').innerText = `⚡ ${d.fps} FPS`;
          }

          // Heap & PSRAM
          if (d.heap !== undefined) {
            const heapKB  = Math.round(d.heap / 1024);
            const psramKB = Math.round(d.psram / 1024);
            document.getElementById('stat-heap').innerText = `🧠 ${heapKB}KB / ${psramKB}KB`;
            document.getElementById('stat-heap').style.color = heapKB < 30 ? 'var(--danger)' : 'var(--text-muted)';
          }

          // SD Card status
          if (d.sd_mounted !== undefined) {
            const sdEl = document.getElementById('stat-sd');
            sdEl.innerText = d.sd_mounted ? '💾 SD ✅' : '💾 SD ❌';
            sdEl.style.color = d.sd_mounted ? '#10b981' : '#ef4444';
          }

          // Recording indicator
          if (d.recording !== undefined) {
            const recEl = document.getElementById('stat-rec');
            recEl.style.display = d.recording ? 'inline-flex' : 'none';
          }
        })
        .catch(() => {
          document.getElementById('live-indicator').classList.remove('active');
          document.getElementById('pill-live-dot').classList.remove('active');
          document.getElementById('stat-status').innerText = 'Reconnecting...';
        });
    }

    // ─── Toast Notifications ────────────────────────────────────
    function showToast(msg) {
      const box = document.getElementById('toast-box');
      const t = document.createElement('div');
      t.className = 'toast-msg';
      t.innerText = msg;
      box.appendChild(t);
      setTimeout(() => t.remove(), 3200);
    }

    // ─── Camera Sensor Controls ─────────────────────────────────
    function updateControl(varName, val) {
      fetch(`/control?var=${varName}&val=${val}`).catch(() => {});
    }

    function toggleSwitch(elId, varName) {
      const el = document.getElementById(elId);
      const isNowActive = !el.classList.contains('active');
      el.classList.toggle('active', isNowActive);
      if (varName) {
        updateControl(varName, isNowActive ? 1 : 0);
      }
    }

    function applyPreset(name) {
      if (name === 'turbo') {
        updateControl('framesize', 6);
        updateControl('fps', 25);
        updateControl('quality', 14);
        document.getElementById('sel-res').value = 6;
        document.getElementById('rng-fps').value = 25;
        document.getElementById('disp-fps').innerText = '25';
        document.getElementById('val-fps-badge').innerText = '25 FPS';
        showToast('⚡ Preset: Turbo 25fps activated');
      } else if (name === 'night') {
        updateControl('framesize', 6);
        updateControl('gainceiling', 6);
        updateControl('aec2', 1);
        updateControl('brightness', 1);
        document.getElementById('sel-gainceiling').value = 6;
        document.getElementById('sw-aec2').classList.add('active');
        document.getElementById('rng-bright').value = 1;
        document.getElementById('disp-bright').innerText = '1';
        showToast('🌙 Preset: Night Vision activated (128x Gain)');
      } else if (name === 'daylight') {
        updateControl('wb_mode', 1);
        updateControl('brightness', 0);
        updateControl('contrast', 1);
        document.getElementById('sel-wb').value = 1;
        document.getElementById('rng-bright').value = 0;
        document.getElementById('disp-bright').innerText = '0';
        document.getElementById('rng-contrast').value = 1;
        document.getElementById('disp-contrast').innerText = '1';
        showToast('☀️ Preset: Daylight Pro activated');
      } else if (name === 'hd') {
        updateControl('framesize', 10);
        updateControl('quality', 10);
        document.getElementById('sel-res').value = 10;
        document.getElementById('rng-quality').value = 10;
        document.getElementById('disp-quality').innerText = '10';
        showToast('📸 Preset: Ultra 2MP HD activated');
      }
    }

    function capturePhoto() {
      showToast('📸 Taking snapshot photo...');
      window.open('/capture', '_blank');
    }

    function toggleFlash() {
      flashState = flashState ? 0 : 1;
      fetch(`/api/system/flash?state=${flashState}`)
        .then(r => r.text())
        .then(st => {
          const isFlashOn = (st === '1');
          document.getElementById('btn-flash').innerText = isFlashOn ? '💡 Flash ON' : '💡 Flash OFF';
          document.getElementById('btn-flash').classList.toggle('btn-accent', isFlashOn);
          document.getElementById('header-btn-flash').classList.toggle('btn-accent', isFlashOn);
          showToast(`💡 Flash light is ${isFlashOn ? 'ON' : 'OFF'}`);
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

    // ─── SD Card File Manager (Fixed Recursive Delete) ───────────
    function toggleViewMode() {
      viewMode = (viewMode === 'grid') ? 'list' : 'grid';
      document.getElementById('btn-view-mode').innerText = (viewMode === 'grid') ? '⊞ Grid' : '☰ List';
      renderFileList();
    }

    function loadStorageInfo() {
      fetch('/api/sdcard/info')
        .then(r => r.json())
        .then(d => {
          if (d.mounted) {
            const usedMB = (d.used / 1024).toFixed(1);
            const totMB = (d.total / 1024).toFixed(1);
            const pct = Math.round((d.used / d.total) * 100) || 0;
            document.getElementById('sd-usage-text').innerText = `${usedMB}MB / ${totMB}MB (${pct}%)`;
            document.getElementById('sd-progress-fill').style.width = `${pct}%`;
          } else {
            document.getElementById('sd-usage-text').innerText = 'No SD Card';
          }
        });
    }

    function loadDirectory(path) {
      currentFmPath = path || '/';
      updateBreadcrumbs(currentFmPath);
      const container = document.getElementById('fm-container');
      container.innerHTML = '<div style="text-align:center;padding:2.5rem;color:var(--text-muted);">Loading files...</div>';

      fetch(`/api/sdcard/list?path=${encodeURIComponent(currentFmPath)}`)
        .then(r => r.json())
        .then(d => {
          currentFmPath = d.path || currentFmPath;
          updateBreadcrumbs(currentFmPath);
          currentFiles = d.files || [];
          renderFileList();
        })
        .catch(() => {
          container.innerHTML = '<div style="text-align:center;padding:2.5rem;color:var(--danger);">Failed to load SD contents</div>';
        });
    }

    function updateBreadcrumbs(path) {
      const bc = document.getElementById('fm-breadcrumbs');
      bc.innerHTML = '<span class="fm-crumb" onclick="loadDirectory(\'/\')">📁 Root</span>';
      if (path === '/' || !path) return;
      const parts = path.split('/').filter(p => p.length > 0);
      let cur = '';
      parts.forEach(p => {
        cur += '/' + p;
        const target = cur;
        bc.innerHTML += ` <span style="color:var(--text-muted);">></span> <span class="fm-crumb" onclick="loadDirectory('${target}')">${p}</span>`;
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
        container.innerHTML = '<div style="text-align:center;padding:3rem;color:var(--text-muted);">📁 Empty Directory</div>';
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
            <div class="fm-card ${isSelected ? 'selected' : ''}" onclick="handleItemClick('${f.path}', ${isDir}, ${isImg})">
              <input type="checkbox" class="fm-card-chk" ${isSelected ? 'checked' : ''} onclick="toggleSelect('${f.path}', event)">
              <div class="fm-card-preview">
                ${isDir ? '📁' : isImg ? `<img src="/api/sdcard/download?name=${encodeURIComponent(f.path)}&inline=1" loading="lazy">` : isVid ? '🎬' : '📄'}
              </div>
              <div class="fm-card-title" title="${f.name}">${f.name}</div>
              <div class="fm-card-meta">${szStr}</div>
              <div class="fm-card-actions" onclick="event.stopPropagation()">
                ${!isDir ? `<a class="btn btn-icon" href="/api/sdcard/download?name=${encodeURIComponent(f.path)}" title="Download" style="width:28px;height:28px;font-size:0.75rem;">⬇️</a>` : ''}
                <button class="btn btn-icon btn-danger" onclick="deleteItem('${f.path}', ${isDir})" title="Delete" style="width:28px;height:28px;font-size:0.75rem;">🗑️</button>
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
          const isImg = f.name.toLowerCase().endsWith('.jpg') || f.name.toLowerCase().endsWith('.jpeg');
          const isVid = f.name.toLowerCase().endsWith('.avi');
          const szStr = isDir ? 'Folder' : (f.size > 1048576) ? (f.size/1048576).toFixed(1)+'MB' : (f.size/1024).toFixed(0)+'KB';

          html += `
            <div class="fm-row ${isSelected ? 'selected' : ''}" onclick="handleItemClick('${f.path}', ${isDir}, ${isImg})">
              <input type="checkbox" class="fm-card-chk" style="position:static;" ${isSelected ? 'checked' : ''} onclick="toggleSelect('${f.path}', event)">
              <span style="font-size:1.25rem;">${isDir ? '📁' : isVid ? '🎬' : '📸'}</span>
              <div class="fm-row-name">${f.name}</div>
              <div class="fm-row-size">${szStr}</div>
              <div style="display:flex;gap:0.3rem;" onclick="event.stopPropagation()">
                ${!isDir ? `<a class="btn btn-icon" href="/api/sdcard/download?name=${encodeURIComponent(f.path)}" title="Download" style="width:28px;height:28px;font-size:0.75rem;">⬇️</a>` : ''}
                <button class="btn btn-icon btn-danger" onclick="deleteItem('${f.path}', ${isDir})" title="Delete" style="width:28px;height:28px;font-size:0.75rem;">🗑️</button>
              </div>
            </div>`;
        });
        html += '</div>';
        container.innerHTML = html;
      }
    }

    function handleItemClick(path, isDir, isImg) {
      if (isDir) {
        loadDirectory(path);
      } else if (isImg) {
        openLightbox(path);
      } else {
        toggleSelect(path);
      }
    }

    function openLightbox(path) {
      activeLightboxPath = path;
      document.getElementById('lb-title').innerText = path.substring(path.lastIndexOf('/') + 1);
      document.getElementById('lb-img').src = `/api/sdcard/download?name=${encodeURIComponent(path)}&inline=1`;
      document.getElementById('lb-dl').href = `/api/sdcard/download?name=${encodeURIComponent(path)}`;
      document.getElementById('lb-meta').innerText = path;
      document.getElementById('modal-lightbox').classList.add('open');
    }

    function closeLightbox() {
      document.getElementById('modal-lightbox').classList.remove('open');
      document.getElementById('lb-img').src = '';
    }

    function deleteLightboxFile() {
      if (!activeLightboxPath) return;
      deleteItem(activeLightboxPath, false);
      closeLightbox();
    }

    function toggleSelect(path, ev) {
      if (ev) ev.stopPropagation();
      if (selectedFiles.has(path)) selectedFiles.delete(path);
      else selectedFiles.add(path);
      updateBatchBar();
      renderFileList();
    }

    function selectAllFiles() {
      if (selectedFiles.size === currentFiles.length) selectedFiles.clear();
      else currentFiles.forEach(f => selectedFiles.add(f.path));
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
        count.innerText = `${selectedFiles.size} selected`;
      } else {
        bar.classList.remove('active');
      }
    }

    function deleteItem(path, isDir) {
      const promptMsg = isDir ? `Permanently delete folder "${path}" and ALL files inside it?` : `Delete "${path}"?`;
      if (!confirm(promptMsg)) return;

      showToast(`🗑️ Deleting ${isDir ? 'folder' : 'file'}...`);
      fetch(`/api/sdcard/delete?name=${encodeURIComponent(path)}`)
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            showToast(`🗑️ Successfully deleted ${path}`);
            loadDirectory(currentFmPath);
            loadStorageInfo();
          } else {
            showToast(`❌ Delete failed: ${d.err || 'error'}`);
          }
        })
        .catch(() => showToast('❌ Network error during delete'));
    }

    function deleteSelectedFiles() {
      if (selectedFiles.size === 0) return;
      if (!confirm(`Permanently delete all ${selectedFiles.size} selected items (including folders and files)?`)) return;

      const names = Array.from(selectedFiles).join(',');
      showToast(`🗑️ Deleting ${selectedFiles.size} items...`);
      fetch(`/api/sdcard/delete?name=${encodeURIComponent(names)}`)
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            showToast(`🗑️ Deleted ${d.deleted || selectedFiles.size} items!`);
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
      if (!confirm('⚠️ CRITICAL WARNING: Erase and reformat ALL files on the SD card?')) return;
      showToast('🧹 Formatting SD card...');
      fetch('/api/sdcard/format')
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            showToast('✅ SD card formatted successfully!');
            loadDirectory('/');
            loadStorageInfo();
          } else showToast('❌ Format failed');
        });
    }

    // ─── XiaoZhi AI Voice & Web Chat ────────────────────────────
    let aiSpeechRec = null;
    let isListening = false;
    let aiVoiceEnabled = true;

    function initSpeech() {
      const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
      if (!SpeechRecognition) return false;
      aiSpeechRec = new SpeechRecognition();
      aiSpeechRec.continuous = false;
      aiSpeechRec.interimResults = true;
      aiSpeechRec.lang = navigator.language || 'en-US';

      aiSpeechRec.onstart = () => {
        isListening = true;
        const b = document.getElementById('btn-mic');
        if (b) {
          b.classList.add('listening');
          b.innerHTML = '🔴';
        }
        document.getElementById('ai-input').placeholder = 'Listening to your voice...';
      };
      aiSpeechRec.onresult = (e) => {
        let text = '';
        for (let i = e.resultIndex; i < e.results.length; ++i) {
          text += e.results[i][0].transcript;
        }
        document.getElementById('ai-input').value = text;
        if (e.results[0].isFinal) {
          sendCustomPrompt();
        }
      };
      aiSpeechRec.onerror = () => { stopMic(); };
      aiSpeechRec.onend = () => { stopMic(); };
      return true;
    }

    function toggleMic() {
      if (!aiSpeechRec && !initSpeech()) {
        showToast('⚠️ Speech recognition not supported by browser. Try Chrome or Edge.');
        return;
      }
      if (isListening) {
        aiSpeechRec.stop();
      } else {
        try {
          aiSpeechRec.start();
        } catch (e) {
          initSpeech();
          aiSpeechRec.start();
        }
      }
    }

    function stopMic() {
      isListening = false;
      const b = document.getElementById('btn-mic');
      if (b) {
        b.classList.remove('listening');
        b.innerHTML = '🎙️';
      }
      document.getElementById('ai-input').placeholder = 'Ask XiaoZhi AI or speak...';
    }

    function toggleVoiceOutput() {
      aiVoiceEnabled = !aiVoiceEnabled;
      const sw = document.getElementById('sw-ai-voice');
      if (sw) sw.classList.toggle('active', aiVoiceEnabled);
      showToast(aiVoiceEnabled ? '🔊 Browser voice output ON' : '🔇 Browser voice output OFF');
    }

    function speakAiResponse(text) {
      if (!aiVoiceEnabled || !window.speechSynthesis) return;
      window.speechSynthesis.cancel();

      // Clean markdown formatting & emojis
      let clean = text.replace(/[*_#`~•]/g, '').replace(/\[.*?\]/g, '').trim();
      if (!clean) return;

      if (clean.length > 150) clean = clean.substring(0, 147) + '...';

      const utter = new SpeechSynthesisUtterance(clean);
      utter.rate = 1.05;
      utter.pitch = 1.0;
      window.speechSynthesis.speak(utter);
    }

    function sendAiPrompt(promptText) {
      const box = document.getElementById('ai-chat-box');

      // Append user bubble
      const u = document.createElement('div');
      u.className = 'chat-bubble user';
      u.innerText = promptText;
      box.appendChild(u);
      box.scrollTop = box.scrollHeight;

      // Send to XiaoZhi AI endpoint
      fetch(`/api/xiaozhi/chat?q=${encodeURIComponent(promptText)}`)
        .then(r => r.json())
        .then(d => {
          const a = document.createElement('div');
          a.className = 'chat-bubble ai';
          a.innerText = d.reply || 'No response from XiaoZhi AI';
          box.appendChild(a);
          box.scrollTop = box.scrollHeight;
          if (d.reply) speakAiResponse(d.reply);
        })
        .catch(() => {
          const a = document.createElement('div');
          a.className = 'chat-bubble ai';
          a.innerText = '❌ Error communicating with XiaoZhi AI';
          box.appendChild(a);
        });
    }

    function sendCustomPrompt() {
      const inp = document.getElementById('ai-input');
      const val = inp.value.trim();
      if (!val) return;
      inp.value = '';
      sendAiPrompt(val);
    }

    // ─── XiaoZhi Pairing & Verification Code ────────────────────
    function updateCodeUI(code, linked) {
      if (code) {
        const cd = document.getElementById('xz-code-display');
        const ci = document.getElementById('xz-code-inline');
        if (cd) cd.innerText = code;
        if (ci) ci.innerText = code;
      }
      const b = document.getElementById('xz-link-badge');
      if (b) {
        if (linked) {
          b.innerText = 'Linked';
          b.style.background = 'rgba(34,197,94,0.2)';
          b.style.color = '#4ade80';
        } else {
          b.innerText = 'Unlinked';
          b.style.background = 'rgba(234,179,8,0.2)';
          b.style.color = '#facc15';
        }
      }
    }

    function announcePairingCode() {
      showToast('📢 Announcing code to Telegram...');
      fetch('/api/xiaozhi/announce', { method: 'POST' })
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            updateCodeUI(d.code, d.linked);
            showToast('✅ Code announced to Telegram via text & voice note!');
          } else {
            showToast('❌ Announcement failed');
          }
        })
        .catch(() => showToast('❌ Network error announcing code'));
    }

    function regenPairingCode() {
      if (!confirm('Generate a new 6-digit XiaoZhi pairing code?')) return;
      showToast('🔄 Generating new code...');
      fetch('/api/xiaozhi/regen', { method: 'POST' })
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            updateCodeUI(d.code, d.linked);
            showToast('✅ New verification code generated & announced!');
          } else {
            showToast('❌ Code generation failed');
          }
        })
        .catch(() => showToast('❌ Network error regenerating code'));
    }

    // ─── XiaoZhi Agent Settings ─────────────────────────────────
    function onXzProviderChange() {
      const sel = document.getElementById('sel-xz-provider');
      if (!sel) return;
      const prov = sel.value;
      const cg = document.getElementById('xz-cloud-group');
      const badge = document.getElementById('xz-model-badge');
      if (prov === 'edge') {
        if (cg) cg.style.display = 'none';
        if (badge) { badge.innerText = 'Edge Engine'; badge.style.color = '#a5b4fc'; }
      } else {
        if (cg) cg.style.display = 'block';
        if (badge) {
          badge.innerText = prov.toUpperCase();
          badge.style.color = '#38bdf8';
        }
      }
    }

    function loadAgentSettings() {
      fetch('/api/xiaozhi/settings')
        .then(r => r.json())
        .then(d => {
          if (!d.ok) return;
          if (d.name) document.getElementById('cfg-xz-name').value = d.name;
          if (d.role) document.getElementById('cfg-xz-role').value = d.role;
          if (d.prompt) document.getElementById('cfg-xz-prompt').value = d.prompt;
          if (d.provider) {
            document.getElementById('sel-xz-provider').value = d.provider;
            onXzProviderChange();
          }
          if (d.lang) document.getElementById('sel-xz-lang').value = d.lang;
          if (d.api_key) document.getElementById('cfg-xz-key').value = d.api_key;
          if (d.api_url) document.getElementById('cfg-xz-url').value = d.api_url;
          if (d.model) document.getElementById('cfg-xz-model').value = d.model;
          if (d.tool_photo !== undefined) document.getElementById('chk-tool-photo').checked = d.tool_photo;
          if (d.tool_flash !== undefined) document.getElementById('chk-tool-flash').checked = d.tool_flash;
          if (d.tool_rec !== undefined) document.getElementById('chk-tool-rec').checked = d.tool_rec;
          if (d.tool_telem !== undefined) document.getElementById('chk-tool-telemetry').checked = d.tool_telem;
        })
        .catch(() => {});
    }

    function saveAgentSettings() {
      showToast('💾 Saving Agent Settings...');
      const params = new URLSearchParams({
        name:       document.getElementById('cfg-xz-name').value,
        role:       document.getElementById('cfg-xz-role').value,
        prompt:     document.getElementById('cfg-xz-prompt').value,
        provider:   document.getElementById('sel-xz-provider').value,
        lang:       document.getElementById('sel-xz-lang').value,
        api_key:    document.getElementById('cfg-xz-key').value,
        api_url:    document.getElementById('cfg-xz-url').value,
        model:      document.getElementById('cfg-xz-model').value,
        tool_photo: document.getElementById('chk-tool-photo').checked ? '1' : '0',
        tool_flash: document.getElementById('chk-tool-flash').checked ? '1' : '0',
        tool_rec:   document.getElementById('chk-tool-rec').checked ? '1' : '0',
        tool_telem: document.getElementById('chk-tool-telemetry').checked ? '1' : '0'
      });
      fetch('/api/xiaozhi/settings', { method: 'POST', body: params.toString() })
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            showToast('✅ XiaoZhi Agent Settings Saved!');
            onXzProviderChange();
          } else {
            showToast('❌ Failed to save agent settings');
          }
        })
        .catch(() => showToast('❌ Network error saving settings'));
    }

    // ─── Telegram Diagnostics ───────────────────────────────────
    function testRawHTTPS() {
      showToast('🔒 Testing Telegram TLS handshake...');
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
            diag.style.color = 'var(--danger)';
            diag.innerText = `❌ TLS Handshake FAILED\nError: ${d.err}\nTime: ${d.time || '--'}`;
            showToast(`❌ TLS Handshake Failed: ${d.err}`);
          }
        })
        .catch(() => {
          diag.style.color = 'var(--danger)';
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

    // ─── Settings, System & OTA ─────────────────────────────────
    function loadSystemSettings() {
      fetch('/api/system')
        .then(r => r.json())
        .then(d => {
          document.getElementById('cfg-mdns').value = d.mdns || '';
          document.getElementById('cfg-ssid').value = d.ssid || '';
          document.getElementById('cfg-tg-token').value = d.tg_token || '';
          document.getElementById('cfg-tg-chat').value = d.tg_chat_id || '';
          if (d.tg_voice !== undefined) {
            const vSw = document.getElementById('sw-tg-voice');
            if (vSw) vSw.classList.toggle('active', d.tg_voice === true);
          }
          if (d.ntp_server1) document.getElementById('cfg-ntp1').value = d.ntp_server1;
          if (d.ntp_offset !== undefined) document.getElementById('cfg-ntp-offset').value = d.ntp_offset;
          if (d.ntp_dst !== undefined) {
            const flip = document.getElementById('sw-dst');
            if (flip) flip.classList.toggle('active', d.ntp_dst === 1);
          }
          if (d.system_time) document.getElementById('cfg-clock-display').innerText = d.system_time;
          if (d.xz_code) updateCodeUI(d.xz_code, d.xz_linked);

          // Recording settings
          if (d.rec_enabled !== undefined) {
            const rSw = document.getElementById('sw-rec-enable');
            if (rSw) rSw.classList.toggle('active', d.rec_enabled === true);
          }
          if (d.rec_interval) {
            const selInt = document.getElementById('sel-rec-interval');
            if (selInt) selInt.value = d.rec_interval;
          }

          // Camera sensor initial values
          if (d.framesize !== undefined) document.getElementById('sel-res').value = d.framesize;
          if (d.fps) {
            document.getElementById('rng-fps').value = d.fps;
            document.getElementById('disp-fps').innerText = d.fps;
            document.getElementById('val-fps-badge').innerText = d.fps + ' FPS';
            document.getElementById('hud-fps').innerText = d.fps + ' FPS';
            document.getElementById('stat-fps').innerText = '⚡ ' + d.fps + ' FPS';
          }
          if (d.quality !== undefined) {
            document.getElementById('rng-quality').value = d.quality;
            document.getElementById('disp-quality').innerText = d.quality;
          }
          if (d.brightness !== undefined) {
            document.getElementById('rng-bright').value = d.brightness;
            document.getElementById('disp-bright').innerText = d.brightness;
          }
          if (d.contrast !== undefined) {
            document.getElementById('rng-contrast').value = d.contrast;
            document.getElementById('disp-contrast').innerText = d.contrast;
          }
          if (d.saturation !== undefined) {
            document.getElementById('rng-sat').value = d.saturation;
            document.getElementById('disp-sat').innerText = d.saturation;
          }
          if (d.special_effect !== undefined) document.getElementById('sel-effect').value = d.special_effect;
          if (d.wb_mode !== undefined) document.getElementById('sel-wb').value = d.wb_mode;
          if (d.gainceiling !== undefined) document.getElementById('sel-gainceiling').value = d.gainceiling;

          // Toggles
          if (d.aec !== undefined) document.getElementById('sw-aec').classList.toggle('active', d.aec === 1);
          if (d.aec2 !== undefined) document.getElementById('sw-aec2').classList.toggle('active', d.aec2 === 1);
          if (d.agc !== undefined) document.getElementById('sw-agc').classList.toggle('active', d.agc === 1);
          if (d.awb_gain !== undefined) document.getElementById('sw-awb-gain').classList.toggle('active', d.awb_gain === 1);
          if (d.bpc !== undefined) document.getElementById('sw-bpc').classList.toggle('active', d.bpc === 1);
          if (d.wpc !== undefined) document.getElementById('sw-wpc').classList.toggle('active', d.wpc === 1);
          if (d.lenc !== undefined) document.getElementById('sw-lenc').classList.toggle('active', d.lenc === 1);
          if (d.vflip !== undefined) document.getElementById('sw-vflip').classList.toggle('active', d.vflip === 1);
          if (d.hmirror !== undefined) document.getElementById('sw-hmirror').classList.toggle('active', d.hmirror === 1);
        });
    }

    function saveSettings() {
      const isDst = document.getElementById('sw-dst').classList.contains('active');
      const isTgVoice = document.getElementById('sw-tg-voice') ? document.getElementById('sw-tg-voice').classList.contains('active') : true;
      const params = new URLSearchParams({
        mdns_name: document.getElementById('cfg-mdns').value,
        wifi_ssid: document.getElementById('cfg-ssid').value,
        wifi_pass: document.getElementById('cfg-pass').value,
        tg_token:  document.getElementById('cfg-tg-token').value,
        tg_chat_id:document.getElementById('cfg-tg-chat').value,
        tg_voice:  isTgVoice ? '1' : '0',
        ntp_server1:document.getElementById('cfg-ntp1').value,
        ntp_offset: document.getElementById('cfg-ntp-offset').value,
        ntp_dst:    isDst ? '1' : '0'
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

    // ─── Initialization ─────────────────────────────────────────
    window.addEventListener('DOMContentLoaded', () => {
      const img = document.getElementById('stream-img');
      img.onerror = onStreamError;
      img.onload  = onStreamLoad;
      startStream();
      pollTelemetry();
      setInterval(pollTelemetry, 2000);
      loadSystemSettings();
      loadAgentSettings();
      switchSection('cam');
    });
  </script>
</body>
</html>
)rawliteral";
