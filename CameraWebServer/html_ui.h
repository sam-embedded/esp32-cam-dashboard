#pragma once
#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
  <meta name="theme-color" content="#111827">
  <title>ESP32-CAM Pro</title>
  <style>
    /* jQuery Mobile Dark Slate Theme System */
    :root {
      --jqm-page-bg: #0b0f19;
      --jqm-bar-bg: linear-gradient(180deg, #1e293b 0%, #0f172a 100%);
      --jqm-bar-border: #334155;
      --jqm-inset-bg: #131d2e;
      --jqm-inset-border: rgba(255, 255, 255, 0.08);
      --jqm-divider-bg: linear-gradient(180deg, #1e293b 0%, #172033 100%);
      --jqm-btn-bg: linear-gradient(180deg, #26354a 0%, #1a2537 100%);
      --jqm-btn-border: #3b4d66;
      --jqm-btn-active: #0284c7;
      --jqm-btn-active-border: #38bdf8;
      --jqm-accent: #0284c7;
      --jqm-accent-glow: rgba(2, 132, 199, 0.4);
      --jqm-text: #f8fafc;
      --jqm-text-muted: #94a3b8;
      --jqm-success: #10b981;
      --jqm-danger: #ef4444;
      --jqm-warning: #f59e0b;
      --radius: 10px;
      --nav-height: 60px;
      --header-height: 52px;
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
      background: var(--jqm-page-bg);
      color: var(--jqm-text);
      min-height: 100dvh;
      display: flex;
      flex-direction: column;
      overflow-x: hidden;
    }

    /* ─── jQuery Mobile Header ─── */
    .ui-header {
      background: var(--jqm-bar-bg);
      border-bottom: 1px solid var(--jqm-bar-border);
      height: var(--header-height);
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 0 0.75rem;
      position: sticky;
      top: 0;
      z-index: 100;
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.4);
    }
    .ui-title {
      font-size: 1.05rem;
      font-weight: 700;
      color: #fff;
      display: flex;
      align-items: center;
      gap: 0.4rem;
      letter-spacing: -0.02em;
      text-shadow: 0 1px 2px rgba(0,0,0,0.6);
    }
    .ui-header-right {
      display: flex;
      align-items: center;
      gap: 0.4rem;
    }

    /* ─── jQuery Mobile Button Primitives ─── */
    .ui-btn {
      background: var(--jqm-btn-bg);
      border: 1px solid var(--jqm-btn-border);
      color: var(--jqm-text);
      padding: 0.45rem 0.8rem;
      border-radius: 8px;
      font-size: 0.85rem;
      font-weight: 600;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 0.35rem;
      text-shadow: 0 1px 1px rgba(0,0,0,0.5);
      box-shadow: 0 1px 3px rgba(0,0,0,0.3), inset 0 1px 0 rgba(255,255,255,0.1);
      transition: all 0.15s ease;
      touch-action: manipulation;
    }
    .ui-btn:active {
      transform: translateY(1px);
      box-shadow: inset 0 2px 4px rgba(0,0,0,0.5);
      background: #151e2d;
    }
    .ui-btn-icon-only {
      width: 38px;
      height: 38px;
      padding: 0;
      font-size: 1.1rem;
      border-radius: 8px;
    }
    .ui-btn-accent {
      background: linear-gradient(180deg, #0284c7 0%, #0369a1 100%);
      border-color: #38bdf8;
      color: #fff;
    }
    .ui-btn-accent:active { background: #0284c7; }
    .ui-btn-success {
      background: linear-gradient(180deg, #10b981 0%, #059669 100%);
      border-color: #34d399;
      color: #fff;
    }
    .ui-btn-danger {
      background: linear-gradient(180deg, #ef4444 0%, #dc2626 100%);
      border-color: #f87171;
      color: #fff;
    }
    .ui-btn-block { width: 100%; }

    /* ─── Persistent Bottom Navbar (jQuery Mobile Style) ─── */
    .ui-navbar {
      background: var(--jqm-bar-bg);
      border-top: 1px solid var(--jqm-bar-border);
      position: fixed;
      bottom: 0;
      left: 0;
      right: 0;
      height: calc(var(--nav-height) + env(safe-area-inset-bottom, 0px));
      padding-bottom: env(safe-area-inset-bottom, 0px);
      z-index: 100;
      display: flex;
      box-shadow: 0 -2px 10px rgba(0,0,0,0.5);
    }
    .ui-nav-item {
      flex: 1;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 3px;
      background: transparent;
      border: none;
      border-right: 1px solid rgba(255,255,255,0.05);
      color: var(--jqm-text-muted);
      cursor: pointer;
      padding: 0.35rem 0;
      transition: all 0.15s ease;
      touch-action: manipulation;
    }
    .ui-nav-item:last-child { border-right: none; }
    .ui-nav-item .nav-icon { font-size: 1.25rem; transition: transform 0.15s; }
    .ui-nav-item .nav-label { font-size: 0.7rem; font-weight: 600; text-transform: uppercase; letter-spacing: 0.03em; }
    .ui-nav-item.ui-btn-active {
      color: #38bdf8;
      background: rgba(2, 132, 199, 0.18);
      box-shadow: inset 0 3px 0 #38bdf8;
    }
    .ui-nav-item.ui-btn-active .nav-icon { transform: scale(1.1); }

    /* ─── Main Content Layout ─── */
    .ui-content {
      flex: 1;
      display: flex;
      flex-direction: column;
      padding-bottom: calc(var(--nav-height) + env(safe-area-inset-bottom, 0px) + 0.5rem);
    }

    /* Tab Panes */
    .tab-pane {
      display: none;
      flex-direction: column;
      flex: 1;
      animation: fadeIn 0.2s ease-out;
    }
    .tab-pane.active { display: flex; }
    @keyframes fadeIn { from { opacity: 0; transform: translateY(4px); } to { opacity: 1; transform: translateY(0); } }

    /* ─── Stream Pane (Live View) ─── */
    .viewport-box {
      position: relative;
      width: 100%;
      background: #000;
      min-height: 240px;
      max-height: 70vh;
      display: flex;
      align-items: center;
      justify-content: center;
      overflow: hidden;
      border-bottom: 1px solid var(--jqm-bar-border);
    }
    #stream-img {
      max-width: 100%;
      max-height: 100%;
      object-fit: contain;
      display: block;
    }

    /* HUD Overlay */
    .hud-overlay {
      position: absolute;
      top: 10px;
      left: 10px;
      background: rgba(15, 23, 42, 0.85);
      backdrop-filter: blur(8px);
      -webkit-backdrop-filter: blur(8px);
      border: 1px solid rgba(255,255,255,0.15);
      border-radius: 8px;
      padding: 0.35rem 0.65rem;
      font-size: 0.75rem;
      display: flex;
      align-items: center;
      gap: 0.45rem;
      pointer-events: none;
      z-index: 10;
      box-shadow: 0 4px 12px rgba(0,0,0,0.5);
    }
    .hud-live-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: var(--jqm-danger);
      transition: all 0.3s;
    }
    .hud-live-dot.active {
      background: var(--jqm-success);
      box-shadow: 0 0 8px var(--jqm-success);
    }

    /* Stream Quick Action Toolbar */
    .stream-toolbar {
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 0.6rem 0.85rem;
      background: var(--jqm-bar-bg);
      border-bottom: 1px solid var(--jqm-bar-border);
      gap: 0.4rem;
      flex-wrap: wrap;
    }
    .stream-tools-left, .stream-tools-right {
      display: flex;
      align-items: center;
      gap: 0.45rem;
    }

    /* Telemetry Pill Grid */
    .telemetry-strip {
      display: flex;
      flex-wrap: wrap;
      gap: 0.4rem;
      padding: 0.65rem 0.85rem;
      background: rgba(19, 29, 46, 0.6);
      border-bottom: 1px solid var(--jqm-inset-border);
    }
    .stat-pill {
      background: rgba(255,255,255,0.04);
      border: 1px solid var(--jqm-inset-border);
      border-radius: 6px;
      padding: 0.25rem 0.55rem;
      font-size: 0.76rem;
      font-weight: 500;
      display: inline-flex;
      align-items: center;
      gap: 0.3rem;
      white-space: nowrap;
    }

    /* ─── jQuery Mobile Inset Listview ─── */
    .ui-listview-inset {
      margin: 0.85rem;
      background: var(--jqm-inset-bg);
      border: 1px solid var(--jqm-inset-border);
      border-radius: var(--radius);
      box-shadow: 0 2px 8px rgba(0,0,0,0.3);
      overflow: hidden;
    }
    .ui-list-divider {
      background: var(--jqm-divider-bg);
      border-bottom: 1px solid var(--jqm-inset-border);
      padding: 0.55rem 0.85rem;
      font-size: 0.75rem;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.05em;
      color: #38bdf8;
      display: flex;
      align-items: center;
      justify-content: space-between;
    }
    .ui-field-contain {
      padding: 0.75rem 0.85rem;
      border-bottom: 1px solid rgba(255,255,255,0.05);
      display: flex;
      flex-direction: column;
      gap: 0.4rem;
    }
    .ui-field-contain:last-child { border-bottom: none; }
    .ui-field-row {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 0.5rem;
    }
    .ui-label {
      font-size: 0.85rem;
      font-weight: 600;
      color: var(--jqm-text);
      display: flex;
      align-items: center;
      gap: 0.35rem;
    }
    .ui-subtext {
      font-size: 0.72rem;
      color: var(--jqm-text-muted);
    }
    .ui-val-badge {
      font-size: 0.8rem;
      font-weight: 700;
      color: #38bdf8;
      background: rgba(56, 189, 248, 0.1);
      padding: 0.15rem 0.5rem;
      border-radius: 4px;
      border: 1px solid rgba(56, 189, 248, 0.25);
    }

    /* ─── Controls: Slider, Select, Input, Flipswitch ─── */
    input[type=range] {
      width: 100%;
      height: 8px;
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
      background: rgba(0,0,0,0.35);
      border: 1px solid var(--jqm-inset-border);
      border-radius: 8px;
      color: #fff;
      padding: 0.6rem 0.75rem;
      font-size: 0.86rem;
      outline: none;
      user-select: text;
    }
    select:focus, input:focus {
      border-color: #38bdf8;
      box-shadow: 0 0 0 2px var(--jqm-accent-glow);
    }

    /* jQuery Mobile Flipswitch */
    .ui-flipswitch {
      display: inline-flex;
      position: relative;
      width: 58px;
      height: 30px;
      background: #1e293b;
      border: 1px solid var(--jqm-btn-border);
      border-radius: 16px;
      cursor: pointer;
      transition: all 0.2s ease;
      flex-shrink: 0;
    }
    .ui-flipswitch input { opacity: 0; width: 0; height: 0; }
    .ui-flipswitch-slider {
      position: absolute;
      top: 2px;
      left: 2px;
      width: 24px;
      height: 24px;
      background: #fff;
      border-radius: 50%;
      transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
      box-shadow: 0 2px 4px rgba(0,0,0,0.4);
    }
    .ui-flipswitch.active {
      background: #0284c7;
      border-color: #38bdf8;
    }
    .ui-flipswitch.active .ui-flipswitch-slider {
      transform: translateX(28px);
      background: #fff;
    }

    /* ─── SD Card File Manager Components ─── */
    .fm-toolbar {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 0.5rem;
      padding: 0.65rem 0.85rem;
      background: rgba(0,0,0,0.25);
      border-bottom: 1px solid var(--jqm-inset-border);
      flex-wrap: wrap;
    }
    .fm-breadcrumbs {
      display: flex;
      align-items: center;
      gap: 0.35rem;
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
    .fm-progress-bar {
      height: 8px;
      background: rgba(255,255,255,0.1);
      border-radius: 4px;
      overflow: hidden;
      margin-top: 0.35rem;
    }
    .fm-progress-fill {
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
      border: 1px solid var(--jqm-inset-border);
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
      height: 90px;
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
      color: var(--jqm-text-muted);
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
    .fm-row-size { font-size: 0.74rem; color: var(--jqm-text-muted); width: 70px; text-align: right; }

    /* ─── Lightbox / Dialog Popup ─── */
    .ui-popup-backdrop {
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
    .ui-popup-backdrop.open { display: flex; }
    .ui-popup {
      background: var(--jqm-page-bg);
      border: 1px solid var(--jqm-btn-border);
      border-radius: 12px;
      width: 100%;
      max-width: 600px;
      overflow: hidden;
      box-shadow: 0 20px 40px rgba(0,0,0,0.8);
      animation: popIn 0.2s cubic-bezier(0.16, 1, 0.3, 1);
    }
    @keyframes popIn {
      from { transform: scale(0.92); opacity: 0; }
      to { transform: scale(1); opacity: 1; }
    }
    .ui-popup-header {
      padding: 0.75rem 1rem;
      background: var(--jqm-bar-bg);
      border-bottom: 1px solid var(--jqm-bar-border);
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-weight: 700;
      font-size: 0.95rem;
    }
    .ui-popup-body {
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
      bottom: calc(var(--nav-height) + env(safe-area-inset-bottom, 0px) + 12px);
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

    /* ─── DESKTOP DASHBOARD OPTIMIZATION ─── */
    @media (min-width: 900px) {
      body {
        height: 100vh;
        overflow: hidden;
      }
      .ui-navbar {
        position: static;
        height: var(--header-height);
        padding-bottom: 0;
        border-top: none;
        border-bottom: 1px solid var(--jqm-bar-border);
        box-shadow: none;
        flex: 1;
        max-width: 580px;
      }
      .ui-header {
        justify-content: flex-start;
        gap: 1.5rem;
      }
      .ui-header-right {
        margin-left: auto;
      }
      .ui-content {
        flex: 1;
        display: grid;
        grid-template-columns: 1.25fr 1fr;
        padding-bottom: 0;
        overflow: hidden;
        height: calc(100vh - var(--header-height));
      }
      /* Left Column: Permanent Live Stream & HUD */
      .desktop-stream-col {
        display: flex !important;
        flex-direction: column;
        border-right: 1px solid var(--jqm-bar-border);
        background: #000;
        height: 100%;
        overflow: hidden;
      }
      .viewport-box {
        flex: 1;
        max-height: unset;
        min-height: unset;
      }
      /* Right Column: Tabbed Management Hub */
      .desktop-hub-col {
        display: flex;
        flex-direction: column;
        height: 100%;
        overflow-y: auto;
        background: var(--jqm-page-bg);
      }
      .desktop-hub-header {
        padding: 0.75rem 1rem;
        background: var(--jqm-bar-bg);
        border-bottom: 1px solid var(--jqm-bar-border);
        display: flex;
        align-items: center;
        justify-content: space-between;
      }
      .tab-pane {
        overflow-y: auto;
        flex: 1;
      }
    }
  </style>
</head>
<body>

  <!-- ─── jQuery Mobile Top App Header ─── -->
  <header class="ui-header">
    <div class="ui-title">
      <span>📷</span> ESP32-CAM Pro
    </div>

    <!-- Desktop Tab Navigation seamlessly embedded in header -->
    <nav class="ui-navbar" id="app-navbar">
      <button class="ui-nav-item ui-btn-active" onclick="switchNavTab('tab-stream')" id="nav-btn-stream">
        <span class="nav-icon">📹</span>
        <span class="nav-label">Live</span>
      </button>
      <button class="ui-nav-item" onclick="switchNavTab('tab-cam')" id="nav-btn-cam">
        <span class="nav-icon">🎛️</span>
        <span class="nav-label">Camera</span>
      </button>
      <button class="ui-nav-item" onclick="switchNavTab('tab-sd')" id="nav-btn-sd">
        <span class="nav-icon">📁</span>
        <span class="nav-label">SD Card</span>
      </button>
      <button class="ui-nav-item" onclick="switchNavTab('tab-tg')" id="nav-btn-tg">
        <span class="nav-icon">🤖</span>
        <span class="nav-label">Telegram</span>
      </button>
      <button class="ui-nav-item" onclick="switchNavTab('tab-sys')" id="nav-btn-sys">
        <span class="nav-icon">⚙️</span>
        <span class="nav-label">System</span>
      </button>
    </nav>

    <div class="ui-header-right">
      <button class="ui-btn ui-btn-icon-only" id="header-btn-flash" onclick="toggleFlash()" title="Toggle Flash LED">💡</button>
      <button class="ui-btn ui-btn-icon-only ui-btn-accent" onclick="capturePhoto()" title="Take Snapshot Photo">📷</button>
    </div>
  </header>

  <!-- ─── Main Content Wrapper ─── -->
  <main class="ui-content">

    <!-- ─── Tab 1: Live Stream (Desktop Left Column / Mobile Primary View) ─── -->
    <section class="tab-pane active desktop-stream-col" id="tab-stream">
      <div class="viewport-box" id="viewport-box">
        <div class="hud-overlay">
          <div class="hud-live-dot" id="live-indicator"></div>
          <span id="hud-status" style="font-weight:700;">LIVE</span>
          <span style="color:var(--jqm-text-muted);">|</span>
          <span id="hud-fps">25 FPS</span>
          <span style="color:var(--jqm-text-muted);">|</span>
          <span id="hud-rssi">📶 -- dBm</span>
        </div>
        <img id="stream-img" src="" alt="ESP32-CAM Stream">
      </div>

      <!-- Quick Action Toolbar Under Video -->
      <div class="stream-toolbar">
        <div class="stream-tools-left">
          <button class="ui-btn ui-btn-accent" onclick="capturePhoto()">📷 Snapshot</button>
          <button class="ui-btn" id="btn-flash" onclick="toggleFlash()">💡 Flash OFF</button>
          <button class="ui-btn" onclick="startStream()">🔄 Reload</button>
        </div>
        <div class="stream-tools-right">
          <button class="ui-btn ui-btn-icon-only" onclick="toggleFullscreen()" title="Fullscreen">⛶</button>
        </div>
      </div>

      <!-- Live Hardware Telemetry Strip -->
      <div class="telemetry-strip">
        <div class="stat-pill" style="color:#38bdf8;">
          <span class="hud-live-dot" id="pill-live-dot"></span>
          <span id="stat-status">Connecting...</span>
        </div>
        <div class="stat-pill" id="stat-fps">⚡ 25 FPS</div>
        <div class="stat-pill" id="stat-rssi">📶 -- dBm</div>
        <div class="stat-pill" id="stat-sd" style="color:#10b981;">💾 SD --</div>
        <div class="stat-pill" id="stat-rec" style="display:none;color:#ef4444;font-weight:700;">🔴 REC</div>
        <div class="stat-pill" id="stat-time" style="color:#38bdf8;">🕒 --</div>
        <div class="stat-pill" id="stat-heap" style="color:var(--jqm-text-muted);">🧠 --</div>
        <div class="stat-pill" id="stat-uptime" style="color:#10b981;">⏱ 0s</div>
        <div class="stat-pill" id="stat-ip" style="color:var(--jqm-text-muted);">🌐 --</div>
      </div>
    </section>

    <!-- ─── Tab 2: Camera & OV2640 Sensor Settings ─── -->
    <section class="tab-pane desktop-hub-col" id="tab-cam">
      <div class="ui-listview-inset">
        <div class="ui-list-divider">
          <span>Resolution & Stream Pacing</span>
          <span class="ui-val-badge" id="val-fps-badge">25 FPS</span>
        </div>
        
        <div class="ui-field-contain">
          <div class="ui-field-row">
            <span class="ui-label">Frame Resolution</span>
          </div>
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

        <div class="ui-field-contain">
          <div class="ui-field-row">
            <span class="ui-label">Stream FPS Limit</span>
            <span class="ui-val-badge" id="disp-fps">25</span>
          </div>
          <input type="range" min="1" max="30" value="25" id="rng-fps" oninput="document.getElementById('disp-fps').innerText=this.value" onchange="updateControl('fps', this.value); document.getElementById('val-fps-badge').innerText=this.value+' FPS';">
        </div>

        <div class="ui-field-contain">
          <div class="ui-field-row">
            <span class="ui-label">JPEG Quality (Lower = Better)</span>
            <span class="ui-val-badge" id="disp-quality">14</span>
          </div>
          <input type="range" min="10" max="63" value="14" id="rng-quality" oninput="document.getElementById('disp-quality').innerText=this.value" onchange="updateControl('quality', this.value)">
        </div>
      </div>

      <div class="ui-listview-inset">
        <div class="ui-list-divider">Picture Adjustments</div>
        
        <div class="ui-field-contain">
          <div class="ui-field-row">
            <span class="ui-label">Brightness</span>
            <span class="ui-val-badge" id="disp-bright">0</span>
          </div>
          <input type="range" min="-2" max="2" value="0" id="rng-bright" oninput="document.getElementById('disp-bright').innerText=this.value" onchange="updateControl('brightness', this.value)">
        </div>

        <div class="ui-field-contain">
          <div class="ui-field-row">
            <span class="ui-label">Contrast</span>
            <span class="ui-val-badge" id="disp-contrast">0</span>
          </div>
          <input type="range" min="-2" max="2" value="0" id="rng-contrast" oninput="document.getElementById('disp-contrast').innerText=this.value" onchange="updateControl('contrast', this.value)">
        </div>

        <div class="ui-field-contain">
          <div class="ui-field-row">
            <span class="ui-label">Saturation</span>
            <span class="ui-val-badge" id="disp-sat">0</span>
          </div>
          <input type="range" min="-2" max="2" value="0" id="rng-sat" oninput="document.getElementById('disp-sat').innerText=this.value" onchange="updateControl('saturation', this.value)">
        </div>

        <div class="ui-field-contain">
          <div class="ui-field-row">
            <span class="ui-label">Special Effect</span>
          </div>
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

        <div class="ui-field-contain">
          <div class="ui-field-row">
            <span class="ui-label">White Balance Mode</span>
          </div>
          <select id="sel-wb" onchange="updateControl('wb_mode', this.value)">
            <option value="0">Auto</option>
            <option value="1">Sunny</option>
            <option value="2">Cloudy</option>
            <option value="3">Office</option>
            <option value="4">Home</option>
          </select>
        </div>

        <div class="ui-field-contain">
          <div class="ui-field-row">
            <span class="ui-label">Vertical Flip</span>
            <div class="ui-flipswitch" id="flip-vflip" onclick="toggleFlipswitch('flip-vflip', 'vflip')">
              <div class="ui-flipswitch-slider"></div>
            </div>
          </div>
        </div>

        <div class="ui-field-contain">
          <div class="ui-field-row">
            <span class="ui-label">Horizontal Mirror</span>
            <div class="ui-flipswitch" id="flip-hmirror" onclick="toggleFlipswitch('flip-hmirror', 'hmirror')">
              <div class="ui-flipswitch-slider"></div>
            </div>
          </div>
        </div>

        <div class="ui-field-contain">
          <button class="ui-btn ui-btn-block ui-btn-success" onclick="saveCameraDefaults()">💾 Save Stream Defaults to Flash</button>
        </div>
      </div>
    </section>

    <!-- ─── Tab 3: SD Card File Explorer & Gallery ─── -->
    <section class="tab-pane desktop-hub-col" id="tab-sd">
      <div class="ui-listview-inset">
        <div class="ui-list-divider">
          <span>SD Card Storage</span>
          <span id="sd-usage-text" class="ui-val-badge">Loading...</span>
        </div>
        <div class="ui-field-contain">
          <div class="fm-progress-bar"><div class="fm-progress-fill" id="sd-progress-fill"></div></div>
        </div>
      </div>

      <div class="ui-listview-inset" style="flex:1;display:flex;flex-direction:column;">
        <div class="fm-toolbar">
          <div class="fm-breadcrumbs" id="fm-breadcrumbs">
            <span class="fm-crumb" onclick="loadDirectory('/')">📁 Root</span>
          </div>
          <div style="display:flex;gap:0.35rem;">
            <button class="ui-btn" id="btn-view-mode" onclick="toggleViewMode()" title="Toggle View">⊞ Grid</button>
            <button class="ui-btn" onclick="navigateUpDir()" title="Parent Directory">⬆️ Up</button>
            <button class="ui-btn" onclick="selectAllFiles()" title="Select All">☑️ All</button>
            <button class="ui-btn" onclick="loadDirectory(currentFmPath)" title="Refresh">🔄</button>
          </div>
        </div>

        <!-- Batch Delete Bar -->
        <div class="batch-bar" id="batch-bar">
          <span id="batch-count" style="font-size:0.82rem;font-weight:700;color:#38bdf8;">0 selected</span>
          <div style="display:flex;gap:0.35rem;">
            <button class="ui-btn ui-btn-danger" onclick="deleteSelectedFiles()">🗑️ Delete</button>
            <button class="ui-btn" onclick="clearSelection()">✕ Cancel</button>
          </div>
        </div>

        <!-- Files Container -->
        <div id="fm-container" style="flex:1;overflow-y:auto;min-height:260px;">
          <div style="text-align:center;padding:2.5rem;color:var(--jqm-text-muted);">Loading files...</div>
        </div>

        <div class="ui-field-contain" style="border-top:1px solid var(--jqm-inset-border);display:flex;justify-content:space-between;align-items:center;">
          <span class="ui-subtext">Format permanently erases SD card</span>
          <button class="ui-btn ui-btn-danger" onclick="formatSDCard()">🧹 Format SD</button>
        </div>
      </div>
    </section>

    <!-- ─── Tab 4: Telegram Server Hub & Diagnostics ─── -->
    <section class="tab-pane desktop-hub-col" id="tab-tg">
      <div class="ui-listview-inset">
        <div class="ui-list-divider">Telegram Bot Configuration</div>
        
        <div class="ui-field-contain">
          <span class="ui-label">Bot Token</span>
          <input type="password" id="cfg-tg-token" placeholder="8967102688:AAHEieQC2_ZHa9ci0DiPsc3O4uLclWdLJ-k">
          <span class="ui-subtext">Obtain from @BotFather on Telegram</span>
        </div>

        <div class="ui-field-contain">
          <span class="ui-label">Authorized Chat IDs</span>
          <input type="text" id="cfg-tg-chat" placeholder="318862528, 987654321">
          <span class="ui-subtext">Comma-separated user or group chat IDs</span>
        </div>

        <div class="ui-field-contain">
          <button class="ui-btn ui-btn-block ui-btn-accent" onclick="saveSettings()">💾 Save Telegram Config</button>
        </div>
      </div>

      <div class="ui-listview-inset">
        <div class="ui-list-divider">🧪 Live TLS & HTTPS Diagnostics</div>

        <div class="ui-field-contain">
          <div style="display:grid;grid-template-columns:1fr 1fr;gap:0.4rem;">
            <button class="ui-btn ui-btn-accent" onclick="testRawHTTPS()">🔒 Test TLS Handshake</button>
            <button class="ui-btn" onclick="sendTelegramTest('msg')">✉️ Send Text Test</button>
          </div>
          <button class="ui-btn ui-btn-block" style="margin-top:0.4rem;" onclick="sendTelegramTest('photo')">📸 Send Photo Test</button>

          <div id="tg-diag-box" style="display:none;margin-top:0.6rem;font-size:0.75rem;background:#060a12;border:1px solid rgba(56,189,248,0.25);padding:0.65rem;border-radius:6px;white-space:pre-wrap;font-family:monospace;color:#38bdf8;"></div>
        </div>

        <div class="ui-field-contain">
          <span class="ui-label">Supported Bot Commands</span>
          <div style="font-size:0.78rem;color:var(--jqm-text-muted);display:flex;flex-direction:column;gap:0.25rem;">
            <div><code>/photo</code> - Capture snapshot and return photo</div>
            <div><code>/flash on</code> | <code>/flash off</code> - Toggle illumination LED</div>
            <div><code>/status</code> - Uptime, WiFi RSSI, Heap & Clock info</div>
            <div><code>/help</code> - List all commands</div>
          </div>
        </div>
      </div>
    </section>

    <!-- ─── Tab 5: System, WiFi, NTP & OTA Updates ─── -->
    <section class="tab-pane desktop-hub-col" id="tab-sys">
      <div class="ui-listview-inset">
        <div class="ui-list-divider">Network & Hostname</div>
        
        <div class="ui-field-contain">
          <span class="ui-label">mDNS Hostname</span>
          <input type="text" id="cfg-mdns" placeholder="esp32cam">
          <span class="ui-subtext">Access via http://esp32cam.local</span>
        </div>

        <div class="ui-field-contain">
          <span class="ui-label">WiFi Network Name (SSID)</span>
          <input type="text" id="cfg-ssid" placeholder="FTTH">
        </div>

        <div class="ui-field-contain">
          <span class="ui-label">WiFi Password</span>
          <input type="password" id="cfg-pass" placeholder="••••••••">
        </div>

        <div class="ui-field-contain">
          <button class="ui-btn ui-btn-block ui-btn-accent" onclick="saveSettings()">💾 Save WiFi & Hostname</button>
        </div>
      </div>

      <div class="ui-listview-inset">
        <div class="ui-list-divider">
          <span>NTP System Clock</span>
          <span class="ui-val-badge" id="cfg-clock-display">--</span>
        </div>

        <div class="ui-field-contain">
          <span class="ui-label">Timezone</span>
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

        <div class="ui-field-contain">
          <div class="ui-field-row">
            <span class="ui-label">Daylight Saving (+1h)</span>
            <div class="ui-flipswitch" id="flip-dst" onclick="toggleFlipswitch('flip-dst')">
              <div class="ui-flipswitch-slider"></div>
            </div>
          </div>
        </div>

        <div class="ui-field-contain">
          <span class="ui-label">Primary NTP Server</span>
          <input type="text" id="cfg-ntp1" placeholder="pool.ntp.org">
        </div>

        <div class="ui-field-contain">
          <button class="ui-btn ui-btn-block ui-btn-success" onclick="saveSettings()">💾 Save & Sync Clock</button>
        </div>
      </div>

      <div class="ui-listview-inset">
        <div class="ui-list-divider">Firmware OTA Flash</div>
        <div class="ui-field-contain">
          <span class="ui-label">Select .bin Firmware File</span>
          <input type="file" id="ota-file" accept=".bin">
          <div class="fm-progress-bar"><div class="fm-progress-fill" id="ota-progress"></div></div>
          <button class="ui-btn ui-btn-block ui-btn-accent" style="margin-top:0.5rem;" onclick="uploadOTA()">⬆️ Flash Firmware Now</button>
        </div>
      </div>

      <div class="ui-listview-inset">
        <div class="ui-list-divider">Device Management</div>
        <div class="ui-field-contain">
          <div style="display:grid;grid-template-columns:1fr 1fr;gap:0.4rem;">
            <button class="ui-btn ui-btn-danger" onclick="restartDevice('soft')">🔄 Soft Reboot</button>
            <button class="ui-btn ui-btn-danger" onclick="restartDevice('erase_nvs')">⚠️ Erase NVS</button>
          </div>
        </div>
      </div>
    </section>

  </main>

  <!-- ─── Lightbox Modal for Media Preview ─── -->
  <div class="ui-popup-backdrop" id="modal-lightbox" onclick="closeLightbox()">
    <div class="ui-popup" onclick="event.stopPropagation()">
      <div class="ui-popup-header">
        <span id="lb-title" style="white-space:nowrap;overflow:hidden;text-overflow:ellipsis;max-width:320px;">Media Viewer</span>
        <button class="ui-btn ui-btn-icon-only" onclick="closeLightbox()" style="width:30px;height:30px;font-size:0.9rem;">✕</button>
      </div>
      <div class="ui-popup-body" id="lb-body">
        <img id="lb-img" src="" alt="Snapshot" style="width:100%;border-radius:8px;object-fit:contain;max-height:55vh;">
        <div style="display:flex;justify-content:space-between;align-items:center;margin-top:0.4rem;">
          <span id="lb-meta" class="ui-subtext">Loading...</span>
          <div style="display:flex;gap:0.4rem;">
            <a class="ui-btn ui-btn-accent" id="lb-dl" href="#" download>⬇️ Download</a>
            <button class="ui-btn ui-btn-danger" id="lb-del" onclick="deleteLightboxFile()">🗑️ Delete</button>
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

    // ─── Format Uptime ──────────────────────────────────────────
    function formatUptime(seconds) {
      const d = Math.floor(seconds / 86400);
      const h = Math.floor((seconds % 86400) / 3600);
      const m = Math.floor((seconds % 3600) / 60);
      const s = seconds % 60;
      if (d > 0) return `${d}d ${h.toString().padStart(2,'0')}h ${m.toString().padStart(2,'0')}m ${s.toString().padStart(2,'0')}s`;
      if (h > 0) return `${h}h ${m.toString().padStart(2,'0')}h ${m.toString().padStart(2,'0')}s`;
      if (m > 0) return `${m}m ${s.toString().padStart(2,'0')}s`;
      return `${s}s`;
    }

    // ─── Navigation Tabs Switcher ───────────────────────────────
    function switchNavTab(tabId) {
      const isDesktop = window.innerWidth >= 900;
      
      // Update nav button active states
      document.querySelectorAll('.ui-nav-item').forEach(btn => {
        btn.classList.toggle('ui-btn-active', btn.id === 'nav-btn-' + tabId.replace('tab-', ''));
      });

      if (isDesktop) {
        // On desktop: stream tab is fixed on left; right column swaps tabs
        ['tab-cam', 'tab-sd', 'tab-tg', 'tab-sys'].forEach(t => {
          const el = document.getElementById(t);
          if (el) el.classList.toggle('active', t === tabId || (tabId === 'tab-stream' && t === 'tab-cam'));
        });
      } else {
        // On mobile: single active tab at a time
        document.querySelectorAll('.tab-pane').forEach(p => {
          p.classList.toggle('active', p.id === tabId);
        });
      }

      if (tabId === 'tab-sd') {
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
            document.getElementById('stat-heap').style.color = heapKB < 30 ? 'var(--jqm-danger)' : 'var(--jqm-text-muted)';
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

    function toggleFlipswitch(elId, varName) {
      const el = document.getElementById(elId);
      const isNowActive = !el.classList.contains('active');
      el.classList.toggle('active', isNowActive);
      if (varName) {
        updateControl(varName, isNowActive ? 1 : 0);
      }
    }

    function capturePhoto() {
      showToast('📸 Taking high-resolution snapshot...');
      window.open('/capture', '_blank');
    }

    function toggleFlash() {
      flashState = flashState ? 0 : 1;
      fetch(`/api/system/flash?state=${flashState}`)
        .then(r => r.text())
        .then(st => {
          const isFlashOn = (st === '1');
          document.getElementById('btn-flash').innerText = isFlashOn ? '💡 Flash ON' : '💡 Flash OFF';
          document.getElementById('btn-flash').classList.toggle('ui-btn-accent', isFlashOn);
          document.getElementById('header-btn-flash').classList.toggle('ui-btn-accent', isFlashOn);
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

    // ─── SD Card File Manager ───────────────────────────────────
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
      container.innerHTML = '<div style="text-align:center;padding:2.5rem;color:var(--jqm-text-muted);">Loading files...</div>';

      fetch(`/api/sdcard/list?path=${encodeURIComponent(currentFmPath)}`)
        .then(r => r.json())
        .then(d => {
          currentFmPath = d.path || currentFmPath;
          updateBreadcrumbs(currentFmPath);
          currentFiles = d.files || [];
          renderFileList();
        })
        .catch(() => {
          container.innerHTML = '<div style="text-align:center;padding:2.5rem;color:var(--jqm-danger);">Failed to load SD contents</div>';
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
        bc.innerHTML += ` <span style="color:var(--jqm-text-muted);">></span> <span class="fm-crumb" onclick="loadDirectory('${target}')">${p}</span>`;
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
        container.innerHTML = '<div style="text-align:center;padding:3rem;color:var(--jqm-text-muted);">📁 Empty Directory</div>';
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
                ${!isDir ? `<a class="ui-btn ui-btn-icon-only" href="/api/sdcard/download?name=${encodeURIComponent(f.path)}" title="Download" style="width:28px;height:28px;font-size:0.75rem;">⬇️</a>` : ''}
                <button class="ui-btn ui-btn-icon-only ui-btn-danger" onclick="deleteSingleFile('${f.path}')" title="Delete" style="width:28px;height:28px;font-size:0.75rem;">🗑️</button>
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
                ${!isDir ? `<a class="ui-btn ui-btn-icon-only" href="/api/sdcard/download?name=${encodeURIComponent(f.path)}" title="Download" style="width:28px;height:28px;font-size:0.75rem;">⬇️</a>` : ''}
                <button class="ui-btn ui-btn-icon-only ui-btn-danger" onclick="deleteSingleFile('${f.path}')" title="Delete" style="width:28px;height:28px;font-size:0.75rem;">🗑️</button>
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
      deleteSingleFile(activeLightboxPath);
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

    function deleteSingleFile(path) {
      if (!confirm(`Delete ${path}?`)) return;
      fetch(`/api/sdcard/delete?name=${encodeURIComponent(path)}`)
        .then(r => r.json())
        .then(d => {
          if (d.ok) {
            showToast('🗑️ File deleted');
            loadDirectory(currentFmPath);
            loadStorageInfo();
          } else showToast('❌ Delete failed');
        });
    }

    function deleteSelectedFiles() {
      if (selectedFiles.size === 0) return;
      if (!confirm(`Delete ${selectedFiles.size} selected items?`)) return;
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
          } else showToast('❌ Batch delete failed');
        });
    }

    function formatSDCard() {
      if (!confirm('⚠️ WARNING: Erase and format ALL files on SD card?')) return;
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
            diag.style.color = 'var(--jqm-danger)';
            diag.innerText = `❌ TLS Handshake FAILED\nError: ${d.err}\nTime: ${d.time || '--'}`;
            showToast(`❌ TLS Handshake Failed: ${d.err}`);
          }
        })
        .catch(() => {
          diag.style.color = 'var(--jqm-danger)';
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
          if (d.ntp_server1) document.getElementById('cfg-ntp1').value = d.ntp_server1;
          if (d.ntp_offset !== undefined) document.getElementById('cfg-ntp-offset').value = d.ntp_offset;
          if (d.ntp_dst !== undefined) {
            const flip = document.getElementById('flip-dst');
            if (flip) flip.classList.toggle('active', d.ntp_dst === 1);
          }
          if (d.system_time) document.getElementById('cfg-clock-display').innerText = d.system_time;

          if (d.fps) {
            document.getElementById('rng-fps').value = d.fps;
            document.getElementById('disp-fps').innerText = d.fps;
            document.getElementById('val-fps-badge').innerText = d.fps + ' FPS';
            document.getElementById('hud-fps').innerText = d.fps + ' FPS';
            document.getElementById('stat-fps').innerText = '⚡ ' + d.fps + ' FPS';
          }
        });
    }

    function saveSettings() {
      const isDst = document.getElementById('flip-dst').classList.contains('active');
      const params = new URLSearchParams({
        mdns_name: document.getElementById('cfg-mdns').value,
        wifi_ssid: document.getElementById('cfg-ssid').value,
        wifi_pass: document.getElementById('cfg-pass').value,
        tg_token:  document.getElementById('cfg-tg-token').value,
        tg_chat_id:document.getElementById('cfg-tg-chat').value,
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

      // Desktop layout sync
      if (window.innerWidth >= 900) {
        switchNavTab('tab-cam');
      }
    });

    window.addEventListener('resize', () => {
      if (window.innerWidth >= 900) {
        document.getElementById('tab-stream').classList.add('active');
      }
    });
  </script>
</body>
</html>
)rawliteral";
