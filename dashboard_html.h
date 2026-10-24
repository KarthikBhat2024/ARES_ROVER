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
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
  <meta name="theme-color" content="#070b14">
  <meta name="description" content="ARES Rover - Mission Control Dashboard & Teleoperation System">
  <title>ARES ROVER // Mission Control</title>
  <style>
    :root {
      --bg-primary: #070b14;
      --bg-surface: rgba(14, 22, 37, 0.75);
      --bg-surface-elevated: rgba(22, 34, 57, 0.85);
      --border-subtle: rgba(56, 189, 248, 0.15);
      --border-glow: rgba(0, 240, 255, 0.4);
      --neon-cyan: #00f0ff;
      --neon-blue: #38bdf8;
      --neon-emerald: #10b981;
      --neon-amber: #f59e0b;
      --neon-crimson: #ef4444;
      --neon-purple: #8b5cf6;
      --text-primary: #f8fafc;
      --text-secondary: #94a3b8;
      --text-muted: #64748b;
      --glow-cyan: 0 0 15px rgba(0, 240, 255, 0.35);
      --glow-red: 0 0 18px rgba(239, 68, 68, 0.45);
      --radius-sm: 8px;
      --radius-md: 14px;
      --radius-lg: 20px;
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      user-select: none;
      -webkit-user-select: none;
      -webkit-touch-callout: none;
      touch-action: manipulation;
    }

    body {
      background: var(--bg-primary);
      background-image: 
        radial-gradient(circle at 50% 0%, rgba(30, 58, 102, 0.3) 0%, transparent 60%),
        radial-gradient(circle at 10% 90%, rgba(16, 185, 129, 0.08) 0%, transparent 40%),
        radial-gradient(circle at 90% 90%, rgba(0, 240, 255, 0.08) 0%, transparent 40%);
      color: var(--text-primary);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen, Ubuntu, Cantarell, "Helvetica Neue", sans-serif;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      overflow-x: hidden;
      padding-bottom: env(safe-area-inset-bottom, 16px);
    }

    /* Top HUD Header */
    header {
      background: rgba(7, 11, 20, 0.92);
      backdrop-filter: blur(16px);
      -webkit-backdrop-filter: blur(16px);
      border-bottom: 1px solid var(--border-subtle);
      padding: 10px 18px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      position: sticky;
      top: 0;
      z-index: 1000;
    }

    .brand-section {
      display: flex;
      align-items: center;
      gap: 12px;
    }

    .brand-badge {
      width: 38px;
      height: 38px;
      background: linear-gradient(135deg, rgba(0, 240, 255, 0.2), rgba(56, 189, 248, 0.05));
      border: 1.5px solid var(--neon-cyan);
      border-radius: var(--radius-sm);
      display: flex;
      align-items: center;
      justify-content: center;
      box-shadow: var(--glow-cyan);
    }

    .brand-badge svg {
      width: 22px;
      height: 22px;
      fill: var(--neon-cyan);
    }

    .brand-text h1 {
      font-size: 15px;
      font-weight: 800;
      letter-spacing: 2px;
      line-height: 1.2;
      background: linear-gradient(90deg, #ffffff 40%, var(--neon-cyan) 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }

    .brand-text p {
      font-size: 10px;
      font-weight: 600;
      color: var(--neon-cyan);
      letter-spacing: 1.2px;
      text-transform: uppercase;
    }

    .hud-telemetry-capsules {
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .capsule {
      background: var(--bg-surface-elevated);
      border: 1px solid var(--border-subtle);
      border-radius: 30px;
      padding: 5px 12px;
      display: flex;
      align-items: center;
      gap: 6px;
      font-size: 11px;
      font-weight: 600;
      transition: all 0.3s;
    }

    .pulse-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: var(--neon-emerald);
      box-shadow: 0 0 8px var(--neon-emerald);
      animation: pulseAnim 2s infinite;
    }

    .pulse-dot.offline {
      background: var(--neon-crimson);
      box-shadow: 0 0 8px var(--neon-crimson);
      animation: none;
    }

    @keyframes pulseAnim {
      0%, 100% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.4; transform: scale(0.85); }
    }

    .btn-help-icon {
      width: 32px;
      height: 32px;
      border-radius: 50%;
      background: rgba(255, 255, 255, 0.08);
      border: 1px solid var(--border-subtle);
      color: var(--neon-cyan);
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: 800;
      font-size: 14px;
      cursor: pointer;
      transition: all 0.2s;
    }

    .btn-help-icon:hover {
      background: var(--neon-cyan);
      color: #070b14;
      box-shadow: var(--glow-cyan);
    }

    /* Main Responsive Grid Layout */
    main {
      flex: 1;
      max-width: 1240px;
      margin: 0 auto;
      width: 100%;
      padding: 14px;
      display: grid;
      grid-template-columns: 360px 1fr;
      gap: 16px;
      align-items: start;
    }

    @media (max-width: 900px) {
      main {
        grid-template-columns: 1fr;
        padding: 10px;
        gap: 12px;
      }
    }

    /* Sleek Cards */
    .card-panel {
      background: var(--bg-surface);
      border: 1px solid var(--border-subtle);
      border-radius: var(--radius-lg);
      padding: 16px;
      backdrop-filter: blur(12px);
      -webkit-backdrop-filter: blur(12px);
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.45);
      position: relative;
      overflow: hidden;
    }

    .card-panel::before {
      content: '';
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      height: 2px;
      background: linear-gradient(90deg, transparent, var(--border-glow), transparent);
    }

    .panel-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 14px;
      padding-bottom: 8px;
      border-bottom: 1px solid rgba(255, 255, 255, 0.06);
    }

    .panel-header-title {
      font-size: 11px;
      font-weight: 700;
      letter-spacing: 1.5px;
      color: var(--neon-cyan);
      text-transform: uppercase;
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .panel-header-tag {
      font-size: 10px;
      background: rgba(255, 255, 255, 0.06);
      padding: 3px 8px;
      border-radius: 12px;
      color: var(--text-secondary);
      font-family: monospace;
    }

    /* Live Telemetry Dash */
    .stats-trio {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 8px;
      margin-bottom: 14px;
    }

    .stat-card {
      background: rgba(7, 12, 22, 0.65);
      border: 1px solid rgba(255, 255, 255, 0.05);
      border-radius: var(--radius-sm);
      padding: 10px 6px;
      text-align: center;
    }

    .stat-card-label {
      font-size: 9px;
      font-weight: 700;
      color: var(--text-muted);
      letter-spacing: 1px;
      text-transform: uppercase;
    }

    .stat-card-value {
      font-size: 15px;
      font-weight: 800;
      color: var(--text-primary);
      margin-top: 4px;
      font-family: monospace;
    }

    /* 6WD Interactive Chassis Visualizer */
    .chassis-container {
      position: relative;
      width: 220px;
      height: 200px;
      margin: 8px auto 14px auto;
      background: radial-gradient(circle, rgba(14, 26, 46, 0.8) 0%, rgba(7, 11, 20, 0.95) 100%);
      border: 1.5px solid rgba(56, 189, 248, 0.2);
      border-radius: var(--radius-md);
      display: flex;
      align-items: center;
      justify-content: center;
      box-shadow: inset 0 0 20px rgba(0, 0, 0, 0.6);
    }

    .rover-hull {
      width: 96px;
      height: 140px;
      background: linear-gradient(180deg, #1e293b 0%, #0f172a 100%);
      border: 2px solid var(--neon-cyan);
      border-radius: 10px;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      position: relative;
      box-shadow: 0 0 16px rgba(0, 240, 255, 0.2);
      transition: transform 0.25s cubic-bezier(0.34, 1.56, 0.64, 1);
    }

    .rover-headlight-beam {
      position: absolute;
      top: -30px;
      width: 70px;
      height: 30px;
      background: radial-gradient(ellipse at bottom, rgba(254, 240, 138, 0.45) 0%, transparent 75%);
      pointer-events: none;
      opacity: 0;
      transition: opacity 0.3s;
    }

    .rover-headlight-beam.active {
      opacity: 1;
    }

    .hull-label {
      font-size: 10px;
      font-weight: 800;
      letter-spacing: 1.5px;
      color: var(--neon-cyan);
      margin-top: 4px;
    }

    .hull-heading-arrow {
      font-size: 20px;
      color: #fff;
      transition: transform 0.3s;
    }

    /* 6 Wheels */
    .wheel-unit {
      position: absolute;
      width: 24px;
      height: 38px;
      background: #1e293b;
      border: 2px solid #475569;
      border-radius: 5px;
      transition: all 0.2s;
    }

    .wheel-unit::after {
      content: '';
      position: absolute;
      left: 2px;
      right: 2px;
      top: 50%;
      height: 2px;
      background: rgba(255, 255, 255, 0.15);
      transform: translateY(-50%);
    }

    .wheel-unit.active-fwd {
      border-color: var(--neon-cyan);
      background: rgba(0, 240, 255, 0.35);
      box-shadow: 0 0 10px var(--neon-cyan);
    }

    .wheel-unit.active-rev {
      border-color: var(--neon-amber);
      background: rgba(245, 158, 11, 0.35);
      box-shadow: 0 0 10px var(--neon-amber);
    }

    /* Wheel Placements */
    .wh-fl { top: 18px; left: -14px; }
    .wh-fr { top: 18px; right: -14px; }
    .wh-ml { top: 78px; left: -14px; }
    .wh-mr { top: 78px; right: -14px; }
    .wh-rl { bottom: 18px; left: -14px; }
    .wh-rr { bottom: 18px; right: -14px; }

    /* Telemetry Log Terminal */
    .terminal-container {
      background: #04070e;
      border: 1px solid rgba(255, 255, 255, 0.07);
      border-radius: var(--radius-sm);
      padding: 8px 10px;
      height: 100px;
      overflow-y: auto;
      font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;
      font-size: 11px;
      color: var(--neon-blue);
      line-height: 1.5;
    }

    .terminal-row {
      margin-bottom: 2px;
      display: flex;
      gap: 6px;
    }

    .terminal-time {
      color: var(--text-muted);
      flex-shrink: 0;
    }

    /* RIGHT PANEL: Modern User-Friendly Controller */
    .controller-tabs {
      display: flex;
      background: rgba(7, 12, 22, 0.8);
      border: 1px solid var(--border-subtle);
      border-radius: 12px;
      padding: 4px;
      gap: 4px;
      margin-bottom: 16px;
    }

    .ctrl-tab-btn {
      flex: 1;
      padding: 8px 12px;
      background: transparent;
      border: none;
      border-radius: 8px;
      color: var(--text-secondary);
      font-size: 11px;
      font-weight: 700;
      letter-spacing: 0.8px;
      cursor: pointer;
      transition: all 0.2s;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 6px;
    }

    .ctrl-tab-btn.active {
      background: var(--bg-surface-elevated);
      color: var(--neon-cyan);
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.3);
    }

    /* Layout 1: Ergonomic Dual-Thumb Gamepad Deck */
    .gamepad-deck {
      display: grid;
      grid-template-columns: 1fr auto 1fr;
      gap: 16px;
      align-items: center;
      justify-items: center;
      padding: 10px 0;
    }

    @media (max-width: 600px) {
      .gamepad-deck {
        gap: 8px;
      }
    }

    .pad-cluster {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 10px;
      width: 100%;
      max-width: 160px;
    }

    .cluster-title {
      font-size: 10px;
      font-weight: 800;
      color: var(--text-muted);
      letter-spacing: 1px;
      text-transform: uppercase;
    }

    /* Giant Center STOP Button */
    .center-safety-core {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 8px;
    }

    .btn-emergency-stop {
      width: 82px;
      height: 82px;
      border-radius: 50%;
      background: linear-gradient(135deg, rgba(239, 68, 68, 0.25) 0%, rgba(185, 28, 28, 0.4) 100%);
      border: 3px solid var(--neon-crimson);
      color: #fff;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      box-shadow: var(--glow-red);
      transition: all 0.15s ease;
      touch-action: manipulation;
    }

    .btn-emergency-stop svg {
      width: 26px;
      height: 26px;
      fill: #fff;
    }

    .btn-emergency-stop span {
      font-size: 10px;
      font-weight: 900;
      letter-spacing: 1px;
      margin-top: 2px;
    }

    .btn-emergency-stop:active {
      transform: scale(0.92);
      background: var(--neon-crimson);
      box-shadow: 0 0 30px var(--neon-crimson);
    }

    /* Modern Driving Touch Controls */
    .btn-touch-drive {
      width: 100%;
      height: 64px;
      background: linear-gradient(180deg, #1b263b 0%, #0f172a 100%);
      border: 1.5px solid #334155;
      border-radius: var(--radius-md);
      color: #fff;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      box-shadow: 0 4px 12px rgba(0, 0, 0, 0.35);
      transition: all 0.1s ease;
      touch-action: manipulation;
    }

    .btn-touch-drive svg {
      width: 24px;
      height: 24px;
      fill: #cbd5e1;
      transition: fill 0.2s;
    }

    .btn-touch-drive span {
      font-size: 10px;
      font-weight: 700;
      color: var(--text-secondary);
      letter-spacing: 0.5px;
      margin-top: 3px;
    }

    .btn-touch-drive:active,
    .btn-touch-drive.active {
      transform: scale(0.94);
      background: linear-gradient(180deg, rgba(0, 240, 255, 0.25) 0%, rgba(14, 165, 233, 0.35) 100%);
      border-color: var(--neon-cyan);
      box-shadow: var(--glow-cyan);
      color: var(--neon-cyan);
    }

    .btn-touch-drive:active svg,
    .btn-touch-drive.active svg {
      fill: var(--neon-cyan);
    }

    /* Lateral Button Pair (Left/Right / Spin) */
    .btn-duo-row {
      display: flex;
      width: 100%;
      gap: 8px;
    }

    .btn-duo-row .btn-touch-drive {
      height: 58px;
    }

    /* Layout 2: Classic 3x3 D-Pad Deck */
    .dpad-matrix-deck {
      display: none;
      flex-direction: column;
      align-items: center;
      gap: 10px;
      padding: 10px 0;
    }

    .dpad-row {
      display: flex;
      gap: 10px;
      justify-content: center;
    }

    .btn-matrix-key {
      width: 72px;
      height: 72px;
      background: linear-gradient(180deg, #1b263b 0%, #0f172a 100%);
      border: 1.5px solid #334155;
      border-radius: var(--radius-md);
      color: #fff;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      box-shadow: 0 4px 14px rgba(0, 0, 0, 0.3);
      transition: all 0.1s ease;
    }

    .btn-matrix-key svg {
      width: 24px;
      height: 24px;
      fill: #cbd5e1;
    }

    .btn-matrix-key span {
      font-size: 9px;
      font-weight: 700;
      color: var(--text-secondary);
      margin-top: 3px;
    }

    .btn-matrix-key:active,
    .btn-matrix-key.active {
      transform: scale(0.93);
      border-color: var(--neon-cyan);
      background: rgba(0, 240, 255, 0.25);
      box-shadow: var(--glow-cyan);
    }

    .btn-matrix-key.stop-key {
      background: rgba(239, 68, 68, 0.18);
      border-color: var(--neon-crimson);
      color: var(--neon-crimson);
    }

    .btn-matrix-key.stop-key svg {
      fill: var(--neon-crimson);
    }

    .btn-matrix-key.stop-key:active {
      background: var(--neon-crimson);
      color: #fff;
    }

    .btn-matrix-key.stop-key:active svg {
      fill: #fff;
    }

    /* Speed Throttle Slider & Speed Modes */
    .throttle-container {
      background: rgba(7, 12, 22, 0.7);
      border: 1px solid var(--border-subtle);
      border-radius: var(--radius-md);
      padding: 14px;
      margin-top: 14px;
    }

    .throttle-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 10px;
    }

    .throttle-title {
      font-size: 11px;
      font-weight: 700;
      letter-spacing: 1px;
      color: var(--text-secondary);
      text-transform: uppercase;
      display: flex;
      align-items: center;
      gap: 6px;
    }

    .throttle-gauge-badge {
      font-family: monospace;
      font-size: 13px;
      font-weight: 800;
      color: var(--neon-cyan);
      background: rgba(0, 240, 255, 0.1);
      padding: 3px 10px;
      border-radius: 20px;
      border: 1px solid rgba(0, 240, 255, 0.3);
    }

    .custom-range-slider {
      width: 100%;
      height: 10px;
      border-radius: 5px;
      background: linear-gradient(90deg, #10b981 0%, #38bdf8 50%, #f59e0b 80%, #ef4444 100%);
      outline: none;
      -webkit-appearance: none;
      cursor: pointer;
    }

    .custom-range-slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 28px;
      height: 28px;
      border-radius: 50%;
      background: #ffffff;
      border: 3px solid var(--neon-cyan);
      box-shadow: 0 0 12px var(--neon-cyan);
      cursor: pointer;
      transition: transform 0.1s;
    }

    .custom-range-slider::-webkit-slider-thumb:active {
      transform: scale(1.15);
    }

    .preset-pills-row {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 8px;
      margin-top: 12px;
    }

    .pill-mode-btn {
      padding: 9px 6px;
      background: rgba(22, 34, 57, 0.7);
      border: 1px solid rgba(255, 255, 255, 0.08);
      border-radius: var(--radius-sm);
      color: var(--text-secondary);
      font-size: 11px;
      font-weight: 700;
      cursor: pointer;
      transition: all 0.2s;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 2px;
    }

    .pill-mode-btn span:first-child {
      font-size: 14px;
    }

    .pill-mode-btn.active {
      background: linear-gradient(135deg, rgba(0, 240, 255, 0.25), rgba(56, 189, 248, 0.1));
      border-color: var(--neon-cyan);
      color: #fff;
      box-shadow: var(--glow-cyan);
    }

    /* Subsystem Action Cards */
    .subsystem-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 10px;
      margin-top: 14px;
    }

    @media (max-width: 600px) {
      .subsystem-grid {
        grid-template-columns: 1fr 1fr;
      }
    }

    .action-toggle-card {
      background: rgba(14, 22, 37, 0.9);
      border: 1px solid rgba(255, 255, 255, 0.08);
      border-radius: var(--radius-md);
      padding: 12px 10px;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 6px;
      cursor: pointer;
      transition: all 0.2s;
    }

    .action-toggle-card svg {
      width: 24px;
      height: 24px;
      fill: var(--text-secondary);
      transition: fill 0.2s;
    }

    .action-toggle-card span {
      font-size: 11px;
      font-weight: 700;
      color: var(--text-secondary);
    }

    .action-toggle-card.active-light {
      border-color: #facc15;
      background: rgba(250, 204, 21, 0.18);
      box-shadow: 0 0 15px rgba(250, 204, 21, 0.35);
    }
    .action-toggle-card.active-light svg { fill: #facc15; }
    .action-toggle-card.active-light span { color: #fef08a; }

    .action-toggle-card.active-beacon {
      border-color: var(--neon-crimson);
      background: rgba(239, 68, 68, 0.18);
      box-shadow: 0 0 15px rgba(239, 68, 68, 0.35);
      animation: beaconFlash 1s infinite alternate;
    }
    .action-toggle-card.active-beacon svg { fill: var(--neon-crimson); }
    .action-toggle-card.active-beacon span { color: #fca5a5; }

    @keyframes beaconFlash {
      from { border-color: rgba(239, 68, 68, 0.4); }
      to { border-color: var(--neon-crimson); box-shadow: 0 0 20px var(--neon-crimson); }
    }

    .action-toggle-card:active {
      transform: scale(0.95);
    }

    /* Modal Instructions Popup */
    .modal-overlay {
      position: fixed;
      top: 0; left: 0; right: 0; bottom: 0;
      background: rgba(4, 7, 14, 0.85);
      backdrop-filter: blur(8px);
      display: none;
      align-items: center;
      justify-content: center;
      z-index: 2000;
      padding: 16px;
    }

    .modal-overlay.open {
      display: flex;
    }

    .modal-box {
      background: #0f172a;
      border: 1px solid var(--border-glow);
      border-radius: var(--radius-lg);
      max-width: 480px;
      width: 100%;
      padding: 20px;
      box-shadow: 0 20px 50px rgba(0, 0, 0, 0.8);
      color: var(--text-primary);
    }

    .modal-head {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 16px;
      border-bottom: 1px solid rgba(255, 255, 255, 0.1);
      padding-bottom: 10px;
    }

    .modal-head h3 {
      font-size: 16px;
      color: var(--neon-cyan);
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .modal-close-btn {
      background: none;
      border: none;
      color: var(--text-muted);
      font-size: 24px;
      cursor: pointer;
    }

    .modal-body ul {
      list-style: none;
      display: flex;
      flex-direction: column;
      gap: 12px;
      font-size: 13px;
    }

    .modal-body li {
      display: flex;
      align-items: flex-start;
      gap: 10px;
      line-height: 1.4;
    }

    .key-badge {
      background: rgba(255, 255, 255, 0.1);
      border: 1px solid rgba(255, 255, 255, 0.2);
      padding: 2px 7px;
      border-radius: 4px;
      font-family: monospace;
      font-weight: 700;
      color: var(--neon-cyan);
    }

    /* Helper Footer info */
    footer {
      text-align: center;
      font-size: 11px;
      color: var(--text-muted);
      margin-top: 10px;
      padding-bottom: 10px;
    }
  </style>
</head>
<body>

  <!-- Top HUD Mission Control Bar -->
  <header>
    <div class="brand-section">
      <div class="brand-badge">
        <svg viewBox="0 0 24 24"><path d="M12 2L2 7v10l10 5 10-5V7L12 2zm0 2.8L19.2 8 12 11.6 4.8 8 12 4.8zM4 9.6l7 3.5v6.5l-7-3.5V9.6zm9 10v-6.5l7-3.5v6.5l-7 3.5z"/></svg>
      </div>
      <div class="brand-text">
        <h1>ARES ROVER</h1>
        <p>6-Wheel Smart Rescue Unit</p>
      </div>
    </div>

    <div class="hud-telemetry-capsules">
      <div class="capsule" id="capsuleConn" title="Wi-Fi Signal & ESP32 Link">
        <div class="pulse-dot" id="pulseDot"></div>
        <span id="connStatusText">ONLINE</span>
      </div>

      <div class="capsule" title="Estimated Power Voltage">
        <span style="color: #facc15;">⚡</span>
        <span id="battDisplay">11.8 V</span>
      </div>

      <button class="btn-help-icon" id="openHelpModal" title="Quick Controls Guide">?</button>
    </div>
  </header>

  <main>
    <!-- LEFT PANEL: Chassis Visualizer, Telemetry & Diagnostics -->
    <section class="card-panel">
      <div class="panel-header">
        <div class="panel-header-title">
          <svg style="width:14px;height:14px;fill:var(--neon-cyan);" viewBox="0 0 24 24"><path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-6h2v6zm0-8h-2V7h2v2z"/></svg>
          Chassis & Telemetry
        </div>
        <div class="panel-header-tag" id="sessionTimer">UP: 00:00:00</div>
      </div>

      <!-- Quick Stats Trio -->
      <div class="stats-trio">
        <div class="stat-card">
          <div class="stat-card-label">DRIVE STATE</div>
          <div class="stat-card-value" id="dispDriveState" style="color: var(--neon-cyan); font-size: 13px;">IDLE</div>
        </div>
        <div class="stat-card">
          <div class="stat-card-label">THROTTLE</div>
          <div class="stat-card-value" id="dispSpeedPercent">63%</div>
        </div>
        <div class="stat-card">
          <div class="stat-card-label">FAILSAFE</div>
          <div class="stat-card-value" style="color: var(--neon-emerald); font-size: 13px;">ACTIVE</div>
        </div>
      </div>

      <!-- 6-Wheel Dynamic Chassis Map -->
      <div class="chassis-container">
        <!-- 6 Wheels -->
        <div class="wheel-unit wh-fl" id="wh-m1" title="Front Left (M1)"></div>
        <div class="wheel-unit wh-fr" id="wh-m2" title="Front Right (M2)"></div>
        <div class="wheel-unit wh-ml" id="wh-m3" title="Middle Left (M3)"></div>
        <div class="wheel-unit wh-mr" id="wh-m4" title="Middle Right (M4)"></div>
        <div class="wheel-unit wh-rl" id="wh-m5" title="Rear Left (M5)"></div>
        <div class="wheel-unit wh-rr" id="wh-m6" title="Rear Right (M6)"></div>

        <!-- Center Chassis Hull -->
        <div class="rover-hull" id="roverHull">
          <div class="rover-headlight-beam" id="headlightBeam"></div>
          <div class="hull-heading-arrow" id="headingArrow">▲</div>
          <div class="hull-label">ARES-6W</div>
        </div>
      </div>

      <!-- Live Event Terminal -->
      <div class="panel-header" style="margin-top: 10px; margin-bottom: 6px;">
        <div class="panel-header-title" style="font-size: 10px; color: var(--text-secondary);">
          EVENT LOG
        </div>
        <div class="panel-header-tag" id="pingIndicator">PING: -- ms</div>
      </div>
      <div class="terminal-container" id="terminalBox">
        <div class="terminal-row">
          <span class="terminal-time">[INIT]</span>
          <span>ARES Rover Dashboard online and ready.</span>
        </div>
      </div>
    </section>

    <!-- RIGHT PANEL: Touch Driving Deck, Throttle & Auxiliaries -->
    <section class="card-panel">
      <!-- Layout Switcher: Dual-Thumb Ergonomic Gamepad vs Classic D-Pad -->
      <div class="controller-tabs">
        <button class="ctrl-tab-btn active" id="tabGamepad" onclick="switchControlLayout('gamepad')">
          <svg style="width:14px;height:14px;fill:currentColor;" viewBox="0 0 24 24"><path d="M21 6H3c-1.1 0-2 .9-2 2v8c0 1.1.9 2 2 2h18c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2zm-10 7H8v3H6v-3H3v-2h3V8h2v3h3v2zm4.5 2c-.83 0-1.5-.67-1.5-1.5s.67-1.5 1.5-1.5 1.5.67 1.5 1.5-.67 1.5-1.5 1.5zm4-3c-.83 0-1.5-.67-1.5-1.5S18.67 9 19.5 9s1.5.67 1.5 1.5-.67 1.5-1.5 1.5z"/></svg>
          DUAL-THUMB PAD
        </button>
        <button class="ctrl-tab-btn" id="tabDpad" onclick="switchControlLayout('dpad')">
          <svg style="width:14px;height:14px;fill:currentColor;" viewBox="0 0 24 24"><path d="M13 3h-2v4h2V3zm-2 18h2v-4h-2v4zm8-10h-4v2h4v-2zm-18 2h4v-2H3v2z"/></svg>
          CLASSIC D-PAD
        </button>
      </div>

      <!-- 1. DUAL-THUMB ERGONOMIC DECK (Mobile-Friendly Gamepad) -->
      <div class="gamepad-deck" id="deckGamepad">
        <!-- Left Thumb: Forward & Reverse Throttle -->
        <div class="pad-cluster">
          <div class="cluster-title">DRIVE AXIS</div>
          <button class="btn-touch-drive btn-control-node" data-cmd="w" id="btn-g-w" title="Drive Forward">
            <svg viewBox="0 0 24 24"><path d="M12 4l-8 8h5v8h6v-8h5z"/></svg>
            <span>FORWARD</span>
          </button>
          <button class="btn-touch-drive btn-control-node" data-cmd="s" id="btn-g-s" title="Drive Reverse">
            <svg viewBox="0 0 24 24"><path d="M12 20l8-8h-5V4h-6v8H4z"/></svg>
            <span>REVERSE</span>
          </button>
        </div>

        <!-- Center: Prominent Red Emergency STOP Button -->
        <div class="center-safety-core">
          <button class="btn-emergency-stop btn-control-node" data-cmd="x" id="btn-g-stop" title="Emergency Stop">
            <svg viewBox="0 0 24 24"><path d="M6 6h12v12H6z"/></svg>
            <span>STOP</span>
          </button>
          <button class="pill-mode-btn" id="btnCruiseToggle" onclick="toggleCruiseLock()" style="padding: 4px 10px; font-size: 10px;" title="Lock Rover in current driving mode">
            <span>🔒</span>
            <span id="cruiseLabel">CRUISE: OFF</span>
          </button>
        </div>

        <!-- Right Thumb: Skid Rotation & Pivot Steering -->
        <div class="pad-cluster">
          <div class="cluster-title">STEER / SPIN</div>
          <div class="btn-duo-row">
            <button class="btn-touch-drive btn-control-node" data-cmd="a" id="btn-g-a" title="Spin Left">
              <svg viewBox="0 0 24 24"><path d="M12.5 8c-2.65 0-5.05 1-6.9 2.6L2 7v9h9l-3.62-3.62c1.39-1.16 3.16-1.88 5.12-1.88 3.54 0 6.55 2.31 7.6 5.5l2.37-.78C21.08 11.03 17.15 8 12.5 8z"/></svg>
              <span>SPIN L</span>
            </button>
            <button class="btn-touch-drive btn-control-node" data-cmd="d" id="btn-g-d" title="Spin Right">
              <svg viewBox="0 0 24 24"><path d="M11.5 8c2.65 0 5.05 1 6.9 2.6L22 7v9h-9l3.62-3.62c-1.39-1.16-3.16-1.88-5.12-1.88-3.54 0-6.55 2.31-7.6 5.5l-2.37-.78C2.92 11.03 6.85 8 11.5 8z"/></svg>
              <span>SPIN R</span>
            </button>
          </div>
          <div class="btn-duo-row">
            <button class="btn-touch-drive btn-control-node" data-cmd="q" id="btn-g-q" title="Pivot Left (Right bank pushes)">
              <svg viewBox="0 0 24 24"><path d="M19 15l-6 6-1.42-1.42L15.17 16H4V4h2v10h9.17l-3.59-3.58L13 9l6 6z"/></svg>
              <span>PIVOT L</span>
            </button>
            <button class="btn-touch-drive btn-control-node" data-cmd="e" id="btn-g-e" title="Pivot Right (Left bank pushes)">
              <svg viewBox="0 0 24 24"><path d="M5 15l6 6 1.42-1.42L8.83 16H20V4h-2v10H8.83l3.59-3.58L11 9 5 15z"/></svg>
              <span>PIVOT R</span>
            </button>
          </div>
        </div>
      </div>

      <!-- 2. CLASSIC 3x3 D-PAD DECK (Alternative Layout) -->
      <div class="dpad-matrix-deck" id="deckDpad">
        <div class="dpad-row">
          <button class="btn-matrix-key btn-control-node" data-cmd="q" id="btn-m-q">
            <svg viewBox="0 0 24 24"><path d="M19 15l-6 6-1.42-1.42L15.17 16H4V4h2v10h9.17l-3.59-3.58L13 9l6 6z"/></svg>
            <span>PIVOT L</span>
          </button>
          <button class="btn-matrix-key btn-control-node" data-cmd="w" id="btn-m-w">
            <svg viewBox="0 0 24 24"><path d="M12 4l-8 8h5v8h6v-8h5z"/></svg>
            <span>FORWARD</span>
          </button>
          <button class="btn-matrix-key btn-control-node" data-cmd="e" id="btn-m-e">
            <svg viewBox="0 0 24 24"><path d="M5 15l6 6 1.42-1.42L8.83 16H20V4h-2v10H8.83l3.59-3.58L11 9 5 15z"/></svg>
            <span>PIVOT R</span>
          </button>
        </div>

        <div class="dpad-row">
          <button class="btn-matrix-key btn-control-node" data-cmd="a" id="btn-m-a">
            <svg viewBox="0 0 24 24"><path d="M12.5 8c-2.65 0-5.05 1-6.9 2.6L2 7v9h9l-3.62-3.62c1.39-1.16 3.16-1.88 5.12-1.88 3.54 0 6.55 2.31 7.6 5.5l2.37-.78C21.08 11.03 17.15 8 12.5 8z"/></svg>
            <span>SPIN L</span>
          </button>
          <button class="btn-matrix-key stop-key btn-control-node" data-cmd="x" id="btn-m-x">
            <svg viewBox="0 0 24 24"><path d="M6 6h12v12H6z"/></svg>
            <span>STOP</span>
          </button>
          <button class="btn-matrix-key btn-control-node" data-cmd="d" id="btn-m-d">
            <svg viewBox="0 0 24 24"><path d="M11.5 8c2.65 0 5.05 1 6.9 2.6L22 7v9h-9l3.62-3.62c-1.39-1.16-3.16-1.88-5.12-1.88-3.54 0-6.55 2.31-7.6 5.5l-2.37-.78C2.92 11.03 6.85 8 11.5 8z"/></svg>
            <span>SPIN R</span>
          </button>
        </div>

        <div class="dpad-row">
          <button class="btn-matrix-key btn-control-node" data-cmd="b" id="btn-m-b">
            <svg viewBox="0 0 24 24"><path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"/></svg>
            <span>BRAKE</span>
          </button>
          <button class="btn-matrix-key btn-control-node" data-cmd="s" id="btn-m-s">
            <svg viewBox="0 0 24 24"><path d="M12 20l8-8h-5V4h-6v8H4z"/></svg>
            <span>REVERSE</span>
          </button>
          <button class="btn-matrix-key btn-control-node" data-cmd="t" id="btn-m-t">
            <svg viewBox="0 0 24 24"><path d="M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58c.18-.14.23-.41.12-.61l-1.92-3.32c-.12-.22-.37-.29-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94l-.36-2.54c-.04-.24-.24-.41-.48-.41h-3.84c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96c-.22-.08-.47 0-.59.22L2.74 8.87c-.12.21-.08.47.12.61l2.03 1.58c-.05.3-.09.63-.09.94s.02.64.07.94l-2.03 1.58c-.18.14-.23.41-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.56 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61l-2.01-1.58zM12 15.6c-1.98 0-3.6-1.62-3.6-3.6s1.62-3.6 3.6-3.6 3.6 1.62 3.6 3.6-1.62 3.6-3.6 3.6z"/></svg>
            <span>DIAG</span>
          </button>
        </div>
      </div>

      <!-- Variable Throttle Speed Control -->
      <div class="throttle-container">
        <div class="throttle-header">
          <div class="throttle-title">
            <svg style="width:14px;height:14px;fill:var(--neon-cyan);" viewBox="0 0 24 24"><path d="M12 4c-4.42 0-8 3.58-8 8s3.58 8 8 8 8-3.58 8-8-3.58-8-8-8zm1 13h-2v-2h2v2zm0-4h-2V7h2v6z"/></svg>
            Motor Throttle PWM
          </div>
          <div class="throttle-gauge-badge" id="lblThrottleVal">160 / 255 (63%)</div>
        </div>
        <input type="range" min="90" max="255" value="160" class="custom-range-slider" id="throttleSlider">

        <!-- 3 Quick Presets -->
        <div class="preset-pills-row">
          <button class="pill-mode-btn" onclick="applyThrottlePreset(105, this)">
            <span>🐢</span>
            <span>CRAWL (41%)</span>
          </button>
          <button class="pill-mode-btn active" onclick="applyThrottlePreset(160, this)">
            <span>⚡</span>
            <span>CRUISE (63%)</span>
          </button>
          <button class="pill-mode-btn" onclick="applyThrottlePreset(255, this)">
            <span>🚀</span>
            <span>TURBO (100%)</span>
          </button>
        </div>
      </div>

      <!-- Auxiliary Subsystems: Headlights, Siren, Self-Test -->
      <div class="subsystem-grid">
        <div class="action-toggle-card" id="cardHeadlights" onclick="toggleHeadlights()">
          <svg viewBox="0 0 24 24"><path d="M9 21c0 .55.45 1 1 1h4c.55 0 1-.45 1-1v-1H9v1zm3-19C8.14 2 5 5.14 5 9c0 2.38 1.19 4.47 3 5.74V17c0 .55.45 1 1 1h6c.55 0 1-.45 1-1v-2.26c1.81-1.27 3-3.36 3-5.74 0-3.86-3.14-7-7-7zm2.85 11.1l-.85.6V16h-4v-1.3l-.85-.6A5.003 5.003 0 0 1 7 9c0-2.76 2.24-5 5-5s5 2.24 5 5c0 1.63-.8 3.16-2.15 4.1z"/></svg>
          <span id="lblLightState">HEADLIGHTS</span>
        </div>

        <div class="action-toggle-card" id="cardBeacon" onclick="toggleRescueBeacon()">
          <svg viewBox="0 0 24 24"><path d="M12 22c1.1 0 2-.9 2-2h-4c0 1.1.9 2 2 2zm6-6v-5c0-3.07-1.63-5.64-4.5-6.32V4c0-.83-.67-1.5-1.5-1.5s-1.5.67-1.5 1.5v.68C7.64 5.36 6 7.92 6 11v5l-2 2v1h16v-1l-2-2zm-2 1H8v-6c0-2.48 1.51-4.5 4-4.5s4 2.02 4 4.5v6z"/></svg>
          <span id="lblBeaconState">BEACON / SIREN</span>
        </div>

        <div class="action-toggle-card" id="cardDiagnostics" onclick="triggerRoverDiagnostics()">
          <svg viewBox="0 0 24 24"><path d="M22 9h-4.79l-4.38-6.56c-.19-.28-.51-.44-.83-.44s-.64.16-.83.44L6.79 9H2c-.55 0-1 .45-1 1s.45 1 1 1h3.21l3.79 5.68V21c0 .55.45 1 1 1s1-.45 1-1v-4.32l3.79-5.68H22c.55 0 1-.45 1-1s-.45-1-1-1z"/></svg>
          <span>RUN DIAGNOSTIC</span>
        </div>
      </div>
    </section>
  </main>

  <footer>
    ARES ROVER TELEOPERATION INTERFACE // ESP32 Wi-Fi Standalone Host
  </footer>

  <!-- Quick Guide Modal -->
  <div class="modal-overlay" id="helpModal">
    <div class="modal-box">
      <div class="modal-head">
        <h3>
          <svg style="width:20px;height:20px;fill:var(--neon-cyan);" viewBox="0 0 24 24"><path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 16h-2v-2h2v2zm1.07-7.75l-.9.92C12.45 11.9 12 12.5 12 14h-2v-.5c0-1.1.45-2.1 1.17-2.83l1.24-1.26c.37-.36.59-.86.59-1.41 0-1.1-.9-2-2-2s-2 .9-2 2H7c0-2.76 2.24-5 5-5s5 2.24 5 5c0 1.04-.42 1.99-1.07 2.75z"/></svg>
          Rover Driver's Handbook
        </h3>
        <button class="modal-close-btn" id="closeHelpModal">&times;</button>
      </div>
      <div class="modal-body">
        <ul>
          <li>
            <span class="key-badge">TOUCH</span>
            <span><strong>Press & Hold</strong> any direction button to drive. Releasing stops the rover immediately.</span>
          </li>
          <li>
            <span class="key-badge">W, A, S, D</span>
            <span>Desktop keyboard shortcuts: <strong>W</strong> (Forward), <strong>S</strong> (Reverse), <strong>A</strong> (Spin Left), <strong>D</strong> (Spin Right).</span>
          </li>
          <li>
            <span class="key-badge">Q, E</span>
            <span>Pivot turning with banking differential drive.</span>
          </li>
          <li>
            <span class="key-badge">SPACE</span>
            <span>Instantly applies hard brake and halts all 6 motors.</span>
          </li>
          <li>
            <span class="key-badge">FAILSAFE</span>
            <span>If Wi-Fi signal drops for more than 1.5 seconds, the safety watchdog automatically halts all motors.</span>
          </li>
        </ul>
      </div>
    </div>
  </div>

  <script>
    /* ==============================================================================
     * ARES MISSION CONTROL - Dynamic Teleoperation Logic
     * ============================================================================== */
    let currentSpeed = 160;
    let activeCommand = null;
    let driveHeartbeatTimer = null;
    let isCruiseLocked = false;
    let lightActive = false;
    let beaconActive = false;
    let isMockMode = (location.protocol === 'file:' || location.hostname === 'localhost' || location.hostname === '127.0.0.1');

    const terminalBox = document.getElementById('terminalBox');
    const dispDriveState = document.getElementById('dispDriveState');
    const dispSpeedPercent = document.getElementById('dispSpeedPercent');
    const throttleSlider = document.getElementById('throttleSlider');
    const lblThrottleVal = document.getElementById('lblThrottleVal');
    const pulseDot = document.getElementById('pulseDot');
    const connStatusText = document.getElementById('connStatusText');
    const battDisplay = document.getElementById('battDisplay');
    const sessionTimer = document.getElementById('sessionTimer');
    const pingIndicator = document.getElementById('pingIndicator');
    const roverHull = document.getElementById('roverHull');
    const headingArrow = document.getElementById('headingArrow');
    const headlightBeam = document.getElementById('headlightBeam');

    function logTerminal(msg, tag = 'SYS') {
      const row = document.createElement('div');
      row.className = 'terminal-row';
      const time = new Date().toTimeString().split(' ')[0];
      row.innerHTML = `<span class="terminal-time">[${time}]</span><span>[${tag}] ${msg}</span>`;
      terminalBox.appendChild(row);
      terminalBox.scrollTop = terminalBox.scrollHeight;
    }

    function triggerHaptic(duration = 20) {
      if ('vibrate' in navigator) {
        try { navigator.vibrate(duration); } catch(e) {}
      }
    }

    // Switch between Dual-Thumb Gamepad & Classic 3x3 Dpad
    function switchControlLayout(layout) {
      const isGamepad = (layout === 'gamepad');
      document.getElementById('deckGamepad').style.display = isGamepad ? 'grid' : 'none';
      document.getElementById('deckDpad').style.display = isGamepad ? 'none' : 'flex';
      document.getElementById('tabGamepad').classList.toggle('active', isGamepad);
      document.getElementById('tabDpad').classList.toggle('active', !isGamepad);
      logTerminal(`Layout changed to ${isGamepad ? 'Dual-Thumb Gamepad' : 'Classic D-Pad'}`);
    }

    // Command Dispatcher to ESP32
    function sendCommand(cmd, spd = currentSpeed) {
      if (isMockMode) {
        simulateMockState(cmd, spd);
        return;
      }
      fetch(`/cmd?c=${encodeURIComponent(cmd)}&spd=${spd}`)
        .then(res => res.json())
        .then(data => updateChassisVisuals(data))
        .catch(err => {
          pulseDot.className = 'pulse-dot offline';
          connStatusText.textContent = 'LINK LOST';
        });
    }

    function startDrivingCommand(cmd) {
      if (activeCommand === cmd) return;
      activeCommand = cmd;
      triggerHaptic(25);
      highlightActiveButtons(cmd, true);
      sendCommand(cmd);

      // Heartbeat pulse every 250ms prevents Watchdog Timeout on ESP32
      clearInterval(driveHeartbeatTimer);
      driveHeartbeatTimer = setInterval(() => {
        if (activeCommand) {
          sendCommand(activeCommand);
        }
      }, 250);
    }

    function stopDrivingCommand() {
      if (isCruiseLocked) return; // Keep running if cruise lock is enabled
      if (!activeCommand) return;
      highlightActiveButtons(activeCommand, false);
      activeCommand = null;
      clearInterval(driveHeartbeatTimer);
      sendCommand('x');
    }

    function highlightActiveButtons(cmd, isActive) {
      document.querySelectorAll(`[data-cmd="${cmd}"]`).forEach(btn => {
        btn.classList.toggle('active', isActive);
      });
    }

    // Attach Event Listeners to all control buttons
    document.querySelectorAll('.btn-control-node').forEach(btn => {
      const cmd = btn.getAttribute('data-cmd');

      // Mouse Events
      btn.addEventListener('mousedown', (e) => {
        e.preventDefault();
        startDrivingCommand(cmd);
      });
      btn.addEventListener('mouseup', (e) => {
        e.preventDefault();
        stopDrivingCommand();
      });
      btn.addEventListener('mouseleave', () => {
        if (!isCruiseLocked && activeCommand === cmd) stopDrivingCommand();
      });

      // Touch Events for Mobile
      btn.addEventListener('touchstart', (e) => {
        e.preventDefault();
        startDrivingCommand(cmd);
      }, { passive: false });

      btn.addEventListener('touchend', (e) => {
        e.preventDefault();
        stopDrivingCommand();
      }, { passive: false });

      btn.addEventListener('touchcancel', () => {
        stopDrivingCommand();
      });
    });

    // Cruise Lock Toggle
    function toggleCruiseLock() {
      isCruiseLocked = !isCruiseLocked;
      triggerHaptic(35);
      const btn = document.getElementById('btnCruiseToggle');
      const label = document.getElementById('cruiseLabel');
      btn.classList.toggle('active', isCruiseLocked);
      label.textContent = `CRUISE: ${isCruiseLocked ? 'LOCKED' : 'OFF'}`;
      logTerminal(`Cruise Lock is now ${isCruiseLocked ? 'ENABLED' : 'DISABLED'}`);
      if (!isCruiseLocked && activeCommand) {
        stopDrivingCommand();
      }
    }

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
        startDrivingCommand(cmd);
      }
    });

    window.addEventListener('keyup', (e) => {
      const key = e.key;
      if (keyMap[key]) {
        stopDrivingCommand();
      }
    });

    // Throttle Slider & Presets
    throttleSlider.addEventListener('input', (e) => {
      currentSpeed = parseInt(e.target.value);
      updateThrottleLabel(currentSpeed);
      document.querySelectorAll('.pill-mode-btn').forEach(p => p.classList.remove('active'));
      if (!isMockMode) fetch(`/speed?val=${currentSpeed}`);
    });

    function applyThrottlePreset(val, btnElem) {
      currentSpeed = val;
      throttleSlider.value = val;
      updateThrottleLabel(val);
      document.querySelectorAll('.pill-mode-btn').forEach(p => p.classList.remove('active'));
      btnElem.classList.add('active');
      triggerHaptic(15);
      if (!isMockMode) fetch(`/speed?val=${currentSpeed}`);
      logTerminal(`Throttle preset applied: ${Math.round((val/255)*100)}%`);
    }

    function updateThrottleLabel(val) {
      const pct = Math.round((val / 255) * 100);
      lblThrottleVal.textContent = `${val} / 255 (${pct}%)`;
      dispSpeedPercent.textContent = pct + '%';
    }

    // Auxiliary Toggles: Lights & Beacon
    function toggleHeadlights() {
      lightActive = !lightActive;
      triggerHaptic(20);
      const card = document.getElementById('cardHeadlights');
      card.classList.toggle('active-light', lightActive);
      headlightBeam.classList.toggle('active', lightActive);
      document.getElementById('lblLightState').textContent = `HEADLIGHTS: ${lightActive ? 'ON' : 'OFF'}`;
      if (!isMockMode) fetch(`/aux?item=light&state=${lightActive ? 1 : 0}`);
      logTerminal(`Headlights ${lightActive ? 'ACTIVATED' : 'DEACTIVATED'}`);
    }

    function toggleRescueBeacon() {
      beaconActive = !beaconActive;
      triggerHaptic(30);
      const card = document.getElementById('cardBeacon');
      card.classList.toggle('active-beacon', beaconActive);
      document.getElementById('lblBeaconState').textContent = `BEACON: ${beaconActive ? 'ON' : 'OFF'}`;
      if (!isMockMode) fetch(`/aux?item=beacon&state=${beaconActive ? 1 : 0}`);
      logTerminal(`Rescue Beacon ${beaconActive ? 'ENGAGED' : 'STANDBY'}`);
    }

    function triggerRoverDiagnostics() {
      triggerHaptic(50);
      logTerminal('Launching hardware diagnostic self-test sequence...', 'TEST');
      sendCommand('t');
    }

    // Dynamic 6-Wheel Visual Feedback
    function updateChassisVisuals(data) {
      if (!data) return;
      const state = data.state || 'IDLE';
      dispDriveState.textContent = state;

      // Update Heading Arrow & Chassis Tilt
      if (state === 'FORWARD') {
        headingArrow.textContent = '▲';
        roverHull.style.transform = 'translateY(-6px)';
      } else if (state === 'REVERSE') {
        headingArrow.textContent = '▼';
        roverHull.style.transform = 'translateY(6px)';
      } else if (state === 'ROTATE_LEFT' || state === 'SPIN_LEFT') {
        headingArrow.textContent = '↺';
        roverHull.style.transform = 'rotate(-8deg)';
      } else if (state === 'ROTATE_RIGHT' || state === 'SPIN_RIGHT') {
        headingArrow.textContent = '↻';
        roverHull.style.transform = 'rotate(8deg)';
      } else if (state === 'TURN_LEFT') {
        headingArrow.textContent = '↰';
        roverHull.style.transform = 'rotate(-4deg)';
      } else if (state === 'TURN_RIGHT') {
        headingArrow.textContent = '↱';
        roverHull.style.transform = 'rotate(4deg)';
      } else {
        headingArrow.textContent = '■';
        roverHull.style.transform = 'none';
      }

      // Wheel Animations
      const leftFwd = (state === 'FORWARD' || state === 'TURN_RIGHT' || state === 'ROTATE_RIGHT');
      const leftRev = (state === 'REVERSE' || state === 'ROTATE_LEFT');
      const rightFwd = (state === 'FORWARD' || state === 'TURN_LEFT' || state === 'ROTATE_LEFT');
      const rightRev = (state === 'REVERSE' || state === 'ROTATE_RIGHT');

      ['m1', 'm3', 'm5'].forEach(id => {
        const el = document.getElementById('wh-' + id);
        el.className = 'wheel-unit wh-' + (id==='m1'?'fl':id==='m3'?'ml':'rl') + (leftFwd ? ' active-fwd' : '') + (leftRev ? ' active-rev' : '');
      });

      ['m2', 'm4', 'm6'].forEach(id => {
        const el = document.getElementById('wh-' + id);
        el.className = 'wheel-unit wh-' + (id==='m2'?'fr':id==='m4'?'mr':'rr') + (rightFwd ? ' active-fwd' : '') + (rightRev ? ' active-rev' : '');
      });
    }

    // Mock Simulation Mode (when testing on PC without ESP32 attached)
    function simulateMockState(cmd, spd) {
      const stateMap = {
        'w': 'FORWARD', 's': 'REVERSE', 'a': 'ROTATE_LEFT', 'd': 'ROTATE_RIGHT',
        'q': 'TURN_LEFT', 'e': 'TURN_RIGHT', 'x': 'IDLE', 'b': 'BRAKED', 't': 'SELF_TEST'
      };
      const st = stateMap[cmd] || 'IDLE';
      updateChassisVisuals({ state: st, speed: spd });
      if (cmd !== 'x') logTerminal(`Drive command executed: [${st}] at ${Math.round((spd/255)*100)}% throttle`);
    }

    // Telemetry Polling (every 350ms)
    setInterval(() => {
      if (isMockMode) {
        pingIndicator.textContent = 'PING: 1 ms (SIM)';
        pulseDot.className = 'pulse-dot';
        connStatusText.textContent = 'ONLINE (SIM)';
        return;
      }

      const t0 = performance.now();
      fetch('/status')
        .then(res => res.json())
        .then(data => {
          const t1 = performance.now();
          pingIndicator.textContent = `PING: ${Math.round(t1 - t0)} ms`;
          pulseDot.className = 'pulse-dot';
          connStatusText.textContent = 'ONLINE';

          if (data.uptime !== undefined) {
            const sec = data.uptime;
            const h = String(Math.floor(sec / 3600)).padStart(2, '0');
            const m = String(Math.floor((sec % 3600) / 60)).padStart(2, '0');
            const s = String(sec % 60).padStart(2, '0');
            sessionTimer.textContent = `UP: ${h}:${m}:${s}`;
          }

          if (data.batt !== undefined) {
            battDisplay.textContent = data.batt.toFixed(1) + ' V' + (data.batt_sim ? ' (SIM)' : '');
          }

          updateChassisVisuals(data);
        })
        .catch(err => {
          pulseDot.className = 'pulse-dot offline';
          connStatusText.textContent = 'RECONNECTING';
        });
    }, 350);

    // Help Modal Triggers
    const helpModal = document.getElementById('helpModal');
    document.getElementById('openHelpModal').addEventListener('click', () => helpModal.classList.add('open'));
    document.getElementById('closeHelpModal').addEventListener('click', () => helpModal.classList.remove('open'));
    helpModal.addEventListener('click', (e) => {
      if (e.target === helpModal) helpModal.classList.remove('open');
    });
  </script>
</body>
</html>
)rawliteral";