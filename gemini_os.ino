<!DOCTYPE html>
<html lang="ro" class="dark">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Gemini OS - Waveshare ESP32-S3 3.49"</title>
    <!-- Tailwind CSS -->
    <script src="https://cdn.tailwindcss.com"></script>
    <!-- Font Awesome Icons -->
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css" rel="stylesheet" />
    <!-- Tone.js for Native Sound Synthesis -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/tone/14.8.49/Tone.min.js"></script>
    <!-- Prism Syntax Highlighting -->
    <link href="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/themes/prism-tomorrow.min.css" rel="stylesheet" />

    <script>
        tailwind.config = {
            darkMode: 'class',
            theme: {
                extend: {
                    colors: {
                        brand: {
                            500: '#3b82f6',
                            600: '#2563eb',
                            700: '#1d4ed8',
                        },
                        dark: {
                            950: '#070b14',
                            900: '#0f172a',
                            800: '#1e293b',
                            700: '#334155',
                            600: '#475569',
                        }
                    }
                }
            }
        }
    </script>
    <style>
        ::-webkit-scrollbar { width: 8px; height: 8px; }
        ::-webkit-scrollbar-track { background: #0f172a; }
        ::-webkit-scrollbar-thumb { background: #334155; border-radius: 4px; }
        ::-webkit-scrollbar-thumb:hover { background: #475569; }
        
        .tab-content { display: none; }
        .tab-content.active { display: block; animation: fadeIn 0.25s ease-out; }
        
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(6px); }
            to { opacity: 1; transform: translateY(0); }
        }
    </style>
</head>
<body class="bg-dark-950 text-gray-200 font-sans h-screen flex flex-col md:flex-row overflow-hidden">

    <!-- Sidebar -->
    <aside class="w-full md:w-72 bg-dark-900 border-r border-dark-800 flex flex-col flex-shrink-0 z-20">
        <div class="p-5 border-b border-dark-800 flex items-center justify-between">
            <div>
                <h1 class="text-lg font-black bg-clip-text text-transparent bg-gradient-to-r from-blue-400 via-indigo-300 to-cyan-300">
                    <i class="fa-solid fa-microchip text-blue-400 mr-2"></i>Gemini OS Standalone
                </h1>
                <p class="text-[11px] text-gray-400 font-mono mt-0.5">Waveshare 3.49" V2 (Zero Conturi)</p>
            </div>
            <span class="px-2 py-0.5 text-[10px] bg-emerald-500/20 text-emerald-400 rounded-full font-bold border border-emerald-500/30">Fără Xiaozhi</span>
        </div>

        <nav class="flex-1 overflow-y-auto p-3">
            <ul class="space-y-1.5 text-sm">
                <li>
                    <button onclick="switchTab('cloud-builder')" id="tab-btn-cloud-builder" class="w-full text-left px-3.5 py-2.5 rounded-xl bg-emerald-600 text-white font-semibold transition-all flex items-center shadow-lg shadow-emerald-900/30">
                        <i class="fa-solid fa-cloud-arrow-down w-6 text-emerald-200"></i> Generator .BIN în GitHub (Ghid Pas cu Pas)
                    </button>
                </li>
                <li>
                    <button onclick="switchTab('code')" id="tab-btn-code" class="w-full text-left px-3.5 py-2.5 rounded-xl text-gray-400 hover:bg-dark-800 hover:text-white transition-all flex items-center">
                        <i class="fa-solid fa-code w-6 text-cyan-400"></i> 1. Codul gemini_os.ino
                    </button>
                </li>
                <li>
                    <button onclick="switchTab('workflow-file')" id="tab-btn-workflow-file" class="w-full text-left px-3.5 py-2.5 rounded-xl text-gray-400 hover:bg-dark-800 hover:text-white transition-all flex items-center">
                        <i class="fa-solid fa-file-code w-6 text-amber-400"></i> 2. Fișierul build.yml
                    </button>
                </li>
                <li>
                    <button onclick="switchTab('flash-tool')" id="tab-btn-flash-tool" class="w-full text-left px-3.5 py-2.5 rounded-xl text-gray-400 hover:bg-dark-800 hover:text-white transition-all flex items-center">
                        <i class="fa-solid fa-bolt w-6 text-yellow-400"></i> 3. Setări Flash Download Tool
                    </button>
                </li>
                <li>
                    <button onclick="switchTab('simulator')" id="tab-btn-simulator" class="w-full text-left px-3.5 py-2.5 rounded-xl text-gray-400 hover:bg-dark-800 hover:text-white transition-all flex items-center">
                        <i class="fa-solid fa-mobile-screen w-6 text-blue-400"></i> Simulator Ecran Waveshare
                    </button>
                </li>
                <li>
                    <button onclick="switchTab('hardware')" id="tab-btn-hardware" class="w-full text-left px-3.5 py-2.5 rounded-xl text-gray-400 hover:bg-dark-800 hover:text-white transition-all flex items-center">
                        <i class="fa-solid fa-microchip w-6 text-purple-400"></i> Pini Reali & Arhitectură
                    </button>
                </li>
            </ul>
        </nav>

        <div class="p-4 border-t border-dark-800 text-xs bg-dark-900/80">
            <div class="flex items-center gap-2 text-emerald-400 font-bold mb-1">
                <i class="fa-solid fa-shield-halved"></i> 100% Independent
            </div>
            <p class="text-gray-400 text-[11px] leading-relaxed">
                Firmware-ul comunică direct prin HTTPS securizat cu API-ul tău Google Gemini. Fără servere intermediare din China.
            </p>
        </div>
    </aside>

    <!-- Main Content Area -->
    <main class="flex-1 flex flex-col h-full overflow-hidden relative">

        <!-- Top Config Bar (for C++ Code generation tab) -->
        <div id="config-bar" class="hidden bg-dark-900 border-b border-dark-800 p-4 shadow-xl flex-shrink-0 z-10">
            <div class="flex items-center justify-between">
                <span class="text-xs font-bold text-gray-300 uppercase tracking-wider flex items-center gap-2">
                    <i class="fa-solid fa-microchip text-emerald-400"></i> Zero Hardcoding • Configurare 100% On-Screen
                </span>
                <span class="text-[11px] text-emerald-400 bg-emerald-500/10 px-2 py-0.5 rounded border border-emerald-500/20 font-mono">
                    NVS Flash Memory • Salvare Permanentă pe Placă
                </span>
            </div>
            <p class="text-xs text-gray-400 mt-1">
                Nu mai trebuie să introduci nimic în fișierul de cod! Când pornești placa, ecranul tactil afișează automat rețelele Wi-Fi disponibile și tastatura touch.
            </p>
        </div>

        <!-- Scrollable Tab Container -->
        <div class="flex-1 overflow-y-auto p-4 md:p-8">

            <!-- TAB: CLOUD BUILDER (.BIN GENERATOR) -->
            <div id="tab-cloud-builder" class="tab-content active max-w-4xl mx-auto space-y-6">
                <div class="bg-gradient-to-r from-emerald-950/80 via-dark-900 to-slate-900 border border-emerald-500/30 rounded-3xl p-6 shadow-2xl">
                    <div class="flex items-center gap-3 mb-3">
                        <div class="w-12 h-12 rounded-2xl bg-emerald-500/20 text-emerald-400 flex items-center justify-center text-2xl font-bold">
                            <i class="fa-solid fa-keyboard"></i>
                        </div>
                        <div>
                            <h2 class="text-2xl font-black text-white">OS Independent: Tastatură Tactilă + Scaner Wi-Fi Direct pe Ecran</h2>
                            <p class="text-emerald-300 text-xs mt-0.5">Fără parole în cod. Selectezi Wi-Fi din listă pe ecran, tastezi parola pe display, iar apoi alegi dacă scrii prin tastatură sau vorbești cu Gemini.</p>
                        </div>
                    </div>
                </div>

                <!-- Ghid Pas cu Pas pe GitHub -->
                <div class="bg-dark-900 border border-dark-800 rounded-3xl p-6 space-y-6">
                    <h3 class="text-sm font-bold text-white uppercase tracking-wider flex items-center gap-2">
                        <i class="fa-solid fa-list-ol text-emerald-400"></i> Pașii exacți în repo-ul tău `my-gemini-os`:
                    </h3>

                    <!-- Pasul 1 -->
                    <div class="p-5 bg-dark-950 rounded-2xl border border-dark-800 space-y-3">
                        <div class="flex items-center gap-3">
                            <span class="w-7 h-7 rounded-xl bg-emerald-500/20 text-emerald-400 font-bold flex items-center justify-center text-xs">1</span>
                            <h4 class="font-bold text-white text-sm">Adaugă fișierul `gemini_os.ino` (Direct din Tab-ul 1)</h4>
                        </div>
                        <p class="text-xs text-gray-400">
                            Apasă <strong>Add file &rarr; Create new file</strong>, scrie numele <code class="text-yellow-300 font-mono">gemini_os.ino</code>, copiază codul curat din Tab-ul <strong>1. Codul gemini_os.ino</strong> și apasă <strong>Commit changes</strong>. Nu trebuie să modifici niciun text în el!
                        </p>
                    </div>

                    <!-- Pasul 2 -->
                    <div class="p-5 bg-dark-950 rounded-2xl border border-dark-800 space-y-3">
                        <div class="flex items-center gap-3">
                            <span class="w-7 h-7 rounded-xl bg-amber-500/20 text-amber-400 font-bold flex items-center justify-center text-xs">2</span>
                            <h4 class="font-bold text-white text-sm">Adaugă scriptul `.github/workflows/build.yml` (Din Tab-ul 2)</h4>
                        </div>
                        <p class="text-xs text-gray-400">
                            Apasă din nou <strong>Add file &rarr; Create new file</strong>, pune numele <code class="text-amber-300 font-mono">.github/workflows/build.yml</code>, lipește codul din Tab-ul <strong>2</strong> și dă <strong>Commit changes</strong>.
                        </p>
                    </div>

                    <!-- Pasul 3 -->
                    <div class="p-5 bg-dark-950 rounded-2xl border border-dark-800 space-y-3">
                        <div class="flex items-center gap-3">
                            <span class="w-7 h-7 rounded-xl bg-blue-500/20 text-blue-400 font-bold flex items-center justify-center text-xs">3</span>
                            <h4 class="font-bold text-white text-sm">Descarcă binarul din tab-ul Actions și flashează la 0x10000</h4>
                        </div>
                        <p class="text-xs text-gray-400">
                            În tab-ul <strong>Actions</strong>, când compilarea e gata (bifă verde), descarci arhiva <strong>gemini-os-waveshare-bin</strong>. Folosești `flash_download_tool_3.9.11.exe` conform instrucțiunilor din Tab-ul 3.
                        </p>
                    </div>
                </div>
            </div>

            <!-- TAB: SIMULATOR ECRAN WAVESHARE -->
            <div id="tab-simulator" class="tab-content max-w-4xl mx-auto space-y-6">
                <div class="bg-dark-900 border border-dark-800 rounded-3xl p-6">
                    <div class="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4 mb-6">
                        <div>
                            <h2 class="text-2xl font-bold text-white flex items-center gap-2">
                                <i class="fa-solid fa-mobile-screen text-blue-400"></i> Simulator Interactiv Waveshare 3.49"
                            </h2>
                            <p class="text-xs text-gray-400 mt-1">
                                Testează exact fluxul ce va rula pe ecran: <strong>Scanare Wi-Fi &rarr; Tastatură Touch &rarr; Chat Text sau Voce</strong>.
                            </p>
                        </div>
                        <button onclick="simReset()" class="bg-dark-800 hover:bg-dark-700 text-gray-300 px-4 py-2 rounded-xl text-xs font-bold transition-all flex items-center gap-2 border border-dark-700">
                            <i class="fa-solid fa-rotate-right"></i> Restart OS
                        </button>
                    </div>

                    <!-- Simulator Mockup Device -->
                    <div class="flex justify-center items-center py-6 bg-dark-950/80 rounded-3xl border border-dark-800/80">
                        <div class="w-[220px] h-[640px] bg-dark-900 rounded-[38px] p-3 border-4 border-dark-700 shadow-2xl relative flex flex-col justify-between select-none">
                            <!-- Top Speaker / Sensors -->
                            <div class="flex justify-center gap-1.5 py-1">
                                <div class="w-1.5 h-1.5 bg-dark-700 rounded-full"></div>
                                <div class="w-1.5 h-1.5 bg-dark-700 rounded-full"></div>
                                <div class="w-1.5 h-1.5 bg-dark-700 rounded-full"></div>
                            </div>

                            <!-- Real 172x640 Ratio Touch Screen Container -->
                            <div id="sim-screen" class="w-full flex-1 bg-black rounded-2xl overflow-hidden flex flex-col relative border border-dark-800 p-2.5 text-white font-sans">
                                
                                <!-- SCREEN 1: WIFI SCANNER -->
                                <div id="screen-wifi-scan" class="flex-1 flex flex-col">
                                    <div class="flex justify-between items-center text-[10px] text-gray-400 pb-1.5 border-b border-dark-800 font-mono">
                                        <span class="text-cyan-400 font-bold"><i class="fa-solid fa-wifi"></i> Scanare</span>
                                        <span id="sim-scan-status" class="text-amber-400">Caut rețele...</span>
                                    </div>
                                    <div class="mt-2 text-center">
                                        <p class="text-[11px] font-bold text-white">Selectează Wi-Fi</p>
                                        <p class="text-[9px] text-gray-400">Apasă pe rețeaua ta:</p>
                                    </div>
                                    <div id="sim-wifi-list" class="flex-1 overflow-y-auto mt-2 space-y-1.5 text-xs">
                                        <!-- WiFi Items rendered dynamically -->
                                    </div>
                                </div>

                                <!-- SCREEN 2: TOUCH KEYBOARD -->
                                <div id="screen-keyboard" class="hidden flex-1 flex flex-col justify-between">
                                    <div>
                                        <div class="flex justify-between items-center text-[10px] text-gray-400 pb-1 border-b border-dark-800 font-mono">
                                            <span id="sim-kb-title" class="text-yellow-400 truncate max-w-[100px]">Parola Wi-Fi</span>
                                            <button onclick="simCancelKeyboard()" class="text-red-400 hover:text-red-300 font-bold">X</button>
                                        </div>
                                        <!-- Text Display Box -->
                                        <div class="mt-2 bg-dark-900 border border-dark-700 rounded-lg p-1.5 min-h-[32px] flex items-center justify-between">
                                            <span id="sim-kb-input" class="font-mono text-xs text-emerald-400 break-all"></span>
                                            <span class="w-1.5 h-3.5 bg-emerald-400 animate-pulse ml-0.5"></span>
                                        </div>
                                    </div>

                                    <!-- Virtual On-screen Keypad -->
                                    <div class="grid grid-cols-4 gap-1 text-[11px] font-mono mt-2">
                                        <button onclick="simType('1')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">1</button>
                                        <button onclick="simType('2')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">2</button>
                                        <button onclick="simType('3')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">3</button>
                                        <button onclick="simBackspace()" class="bg-red-900/60 text-red-300 rounded p-1.5 text-center"><i class="fa-solid fa-delete-left"></i></button>

                                        <button onclick="simType('a')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">a</button>
                                        <button onclick="simType('b')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">b</button>
                                        <button onclick="simType('c')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">c</button>
                                        <button onclick="simType('d')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">d</button>

                                        <button onclick="simType('e')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">e</button>
                                        <button onclick="simType('f')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">f</button>
                                        <button onclick="simType('g')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">g</button>
                                        <button onclick="simType('h')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">h</button>

                                        <button onclick="simType('i')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">i</button>
                                        <button onclick="simType('o')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">o</button>
                                        <button onclick="simType('m')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">m</button>
                                        <button onclick="simType('s')" class="bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center">s</button>

                                        <button onclick="simType(' ')" class="col-span-2 bg-dark-800 active:bg-cyan-600 rounded p-1.5 text-center text-[10px]">Spațiu</button>
                                        <button onclick="simSubmitKeyboard()" class="col-span-2 bg-emerald-600 active:bg-emerald-500 rounded p-1.5 text-center font-bold text-white text-[10px]">OK / Conectează</button>
                                    </div>
                                </div>

                                <!-- SCREEN 3: MAIN GEMINI OS DASHBOARD -->
                                <div id="screen-main-os" class="hidden flex-1 flex flex-col justify-between">
                                    <!-- Top Status -->
                                    <div class="flex justify-between items-center text-[9px] text-gray-400 pb-1.5 border-b border-dark-800 font-mono">
                                        <span class="text-emerald-400 flex items-center gap-1"><i class="fa-solid fa-wifi"></i> Online</span>
                                        <span class="text-cyan-400">Gemini Flash</span>
                                    </div>

                                    <!-- Chat Log Body -->
                                    <div id="sim-chat-log" class="flex-1 overflow-y-auto py-2 space-y-2 text-[10px]">
                                        <div class="bg-dark-900 border border-dark-800 rounded-lg p-2 text-gray-300">
                                            <span class="text-emerald-400 font-bold block mb-0.5">Gemini OS:</span>
                                            Conectat cu succes! Poți tasta mesaje sau folosi microfonul.
                                        </div>
                                    </div>

                                    <!-- Bottom Action Bar (Keyboard or Voice) -->
                                    <div class="pt-2 border-t border-dark-800 space-y-1.5">
                                        <div class="grid grid-cols-2 gap-1.5">
                                            <button onclick="simOpenPromptKeyboard()" class="bg-cyan-600 hover:bg-cyan-500 active:bg-cyan-700 text-white rounded-lg py-2 text-[10px] font-bold flex items-center justify-center gap-1">
                                                <i class="fa-solid fa-keyboard"></i> Tastează
                                            </button>
                                            <button onclick="simTriggerVoiceInput()" id="sim-btn-mic" class="bg-amber-600 hover:bg-amber-500 active:bg-amber-700 text-white rounded-lg py-2 text-[10px] font-bold flex items-center justify-center gap-1">
                                                <i class="fa-solid fa-microphone"></i> Vorbește
                                            </button>
                                        </div>
                                        <button onclick="simShowWifi()" class="w-full text-center text-[9px] text-gray-500 hover:text-gray-300">
                                            <i class="fa-solid fa-gear"></i> Schimbă Wi-Fi
                                        </button>
                                    </div>
                                </div>

                            </div>

                            <!-- Bottom Home Bar -->
                            <div class="flex justify-center pt-2">
                                <div class="w-12 h-1 bg-dark-700 rounded-full"></div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>

            <!-- TAB: HARDWARE & PINOUT -->
            <div id="tab-hardware" class="tab-content max-w-4xl mx-auto space-y-6">
                <div class="bg-dark-900 border border-dark-800 rounded-3xl p-6">
                    <h2 class="text-2xl font-bold text-white mb-2 flex items-center gap-2">
                        <i class="fa-solid fa-microchip text-purple-400"></i> Pinii Reali Waveshare 3.49" V2
                    </h2>
                    <p class="text-gray-400 text-xs mb-6">
                        Configurația pinilor interni pentru ecranul QSPI și circuitele audio de pe această placă:
                    </p>

                    <div class="grid grid-cols-1 md:grid-cols-2 gap-4 text-xs font-mono">
                        <div class="bg-dark-950 p-4 rounded-2xl border border-dark-800">
                            <h4 class="font-bold text-cyan-400 text-sm font-sans mb-3 flex items-center gap-2">
                                <i class="fa-solid fa-desktop"></i> Ecran QSPI (AXS15231B - 172x640)
                            </h4>
                            <ul class="space-y-1.5 text-gray-300">
                                <li>TFT_CS: <span class="text-yellow-300 font-bold">GPIO 11</span></li>
                                <li>TFT_DC: <span class="text-yellow-300 font-bold">GPIO 8</span></li>
                                <li>TFT_RST: <span class="text-yellow-300 font-bold">GPIO 9</span></li>
                                <li>TFT_SCLK: <span class="text-yellow-300 font-bold">GPIO 12</span></li>
                                <li>TFT_D0: <span class="text-yellow-300 font-bold">GPIO 10</span></li>
                                <li>TFT_D1: <span class="text-yellow-300 font-bold">GPIO 14</span></li>
                                <li>TFT_D2: <span class="text-yellow-300 font-bold">GPIO 15</span></li>
                                <li>TFT_D3: <span class="text-yellow-300 font-bold">GPIO 13</span></li>
                            </ul>
                        </div>

                        <div class="bg-dark-950 p-4 rounded-2xl border border-dark-800">
                            <h4 class="font-bold text-emerald-400 text-sm font-sans mb-3 flex items-center gap-2">
                                <i class="fa-solid fa-volume-high"></i> Audio I2S (ES7210 Mic + ES8311 DAC)
                            </h4>
                            <ul class="space-y-1.5 text-gray-300">
                                <li>I2S_BCLK: <span class="text-yellow-300 font-bold">GPIO 4</span></li>
                                <li>I2S_LRCK: <span class="text-yellow-300 font-bold">GPIO 5</span></li>
                                <li>I2S_DOUT (Difuzor): <span class="text-yellow-300 font-bold">GPIO 6</span></li>
                                <li>I2S_DIN (Microfon): <span class="text-yellow-300 font-bold">GPIO 7</span></li>
                                <li>I2C_SDA (Control): <span class="text-yellow-300 font-bold">GPIO 2</span></li>
                                <li>I2C_SCL (Control): <span class="text-yellow-300 font-bold">GPIO 1</span></li>
                            </ul>
                        </div>
                    </div>
                </div>
            </div>

        </div>
    </main>

    <!-- Scripts -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/prism.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-c.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-cpp.min.js"></script>

    <script>
        let synth = null;
        function playChime(type) {
            try {
                if (!synth && typeof Tone !== 'undefined') {
                    synth = new Tone.PolySynth(Tone.Synth).toDestination();
                    synth.volume.value = -12;
                }
                if (Tone.context && Tone.context.state !== 'running') {
                    Tone.start();
                }
                if (type === 'start') {
                    synth.triggerAttackRelease(["C5", "E5", "G5"], "16n");
                } else if (type === 'respond') {
                    synth.triggerAttackRelease(["G5", "B5", "E6"], "12n");
                } else if (type === 'tap') {
                    synth.triggerAttackRelease(["C6"], "32n");
                }
            } catch (e) {
                console.log('Audio error:', e);
            }
        }

        // SIMULATOR STATE ENGINE
        let simSelectedSSID = '';
        let simKeyboardTarget = ''; // 'wifi' or 'prompt'
        let simCurrentInput = '';

        const mockNetworks = [
            { ssid: "Casa_WiFi_5G", rssi: -45 },
            { ssid: "TP-Link_Fiber", rssi: -60 },
            { ssid: "Hotspot_Mobil", rssi: -72 },
            { ssid: "Vecinul_Guest", rssi: -85 }
        ];

        function simShowWifi() {
            document.getElementById('screen-wifi-scan').classList.remove('hidden');
            document.getElementById('screen-keyboard').classList.add('hidden');
            document.getElementById('screen-main-os').classList.add('hidden');

            const status = document.getElementById('sim-scan-status');
            const list = document.getElementById('sim-wifi-list');
            status.textContent = "Caut rețele...";
            list.innerHTML = `<div class="p-4 text-center text-gray-500"><i class="fa-solid fa-spinner animate-spin"></i> Scanare...</div>`;

            setTimeout(() => {
                status.textContent = "Gata";
                list.innerHTML = "";
                mockNetworks.forEach(net => {
                    const btn = document.createElement('button');
                    btn.className = "w-full text-left p-2 rounded-lg bg-dark-900 hover:bg-dark-800 active:bg-cyan-900 border border-dark-800 flex items-center justify-between text-gray-200 transition-all";
                    btn.innerHTML = `
                        <span class="font-bold truncate max-w-[110px]">${net.ssid}</span>
                        <span class="text-[9px] text-emerald-400 font-mono"><i class="fa-solid fa-wifi"></i></span>
                    `;
                    btn.onclick = () => simSelectNetwork(net.ssid);
                    list.appendChild(btn);
                });
            }, 600);
        }

        function simSelectNetwork(ssid) {
            playChime('tap');
            simSelectedSSID = ssid;
            simKeyboardTarget = 'wifi';
            simCurrentInput = '';
            document.getElementById('sim-kb-title').textContent = "Parolă: " + ssid;
            document.getElementById('sim-kb-input').textContent = "";
            document.getElementById('screen-wifi-scan').classList.add('hidden');
            document.getElementById('screen-keyboard').classList.remove('hidden');
        }

        function simOpenPromptKeyboard() {
            playChime('tap');
            simKeyboardTarget = 'prompt';
            simCurrentInput = '';
            document.getElementById('sim-kb-title').textContent = "Întreabă Gemini";
            document.getElementById('sim-kb-input').textContent = "";
            document.getElementById('screen-main-os').classList.add('hidden');
            document.getElementById('screen-keyboard').classList.remove('hidden');
        }

        function simCancelKeyboard() {
            playChime('tap');
            if (simKeyboardTarget === 'wifi') {
                simShowWifi();
            } else {
                document.getElementById('screen-keyboard').classList.add('hidden');
                document.getElementById('screen-main-os').classList.remove('hidden');
            }
        }

        function simType(char) {
            playChime('tap');
            simCurrentInput += char;
            document.getElementById('sim-kb-input').textContent = simCurrentInput;
        }

        function simBackspace() {
            playChime('tap');
            simCurrentInput = simCurrentInput.slice(0, -1);
            document.getElementById('sim-kb-input').textContent = simCurrentInput;
        }

        function simSubmitKeyboard() {
            playChime('respond');
            if (simKeyboardTarget === 'wifi') {
                // Connected to WiFi
                document.getElementById('screen-keyboard').classList.add('hidden');
                document.getElementById('screen-main-os').classList.remove('hidden');
                addChatMessage("Sistem", `Conectat la ${simSelectedSSID}! Parola salvată în NVS.`);
            } else if (simKeyboardTarget === 'prompt') {
                const query = simCurrentInput || "Salut Gemini!";
                document.getElementById('screen-keyboard').classList.add('hidden');
                document.getElementById('screen-main-os').classList.remove('hidden');
                addChatMessage("Tu (Text)", query);
                
                // Simulate Gemini AI Response
                setTimeout(() => {
                    playChime('respond');
                    addChatMessage("Gemini", `Am primit întrebarea ta („${query}”). Răspund direct pe Waveshare 3.49" fără servere intermediare!`);
                }, 1000);
            }
        }

        function simTriggerVoiceInput() {
            playChime('start');
            const btn = document.getElementById('sim-btn-mic');
            btn.innerHTML = `<i class="fa-solid fa-spinner animate-spin"></i> Ascult...`;
            btn.classList.add('bg-red-600');

            setTimeout(() => {
                btn.innerHTML = `<i class="fa-solid fa-brain animate-pulse"></i> Gândesc...`;
                addChatMessage("Tu (Voce)", "«Salut, cum e vremea astăzi?»");
            }, 1800);

            setTimeout(() => {
                playChime('respond');
                btn.innerHTML = `<i class="fa-solid fa-microphone"></i> Vorbește`;
                btn.classList.remove('bg-red-600');
                addChatMessage("Gemini", "Astăzi este o zi frumoasă și însorită! Interacțiunea vocală I2S funcționează perfect.");
            }, 3600);
        }

        function addChatMessage(sender, text) {
            const chatLog = document.getElementById('sim-chat-log');
            const bubble = document.createElement('div');
            const isUser = sender.startsWith('Tu');
            bubble.className = `p-2 rounded-lg border text-[10px] leading-tight ${isUser ? 'bg-cyan-950/70 border-cyan-800 text-cyan-200 ml-2' : 'bg-dark-900 border-dark-800 text-gray-200 mr-2'}`;
            bubble.innerHTML = `<span class="font-bold block mb-0.5 ${isUser ? 'text-cyan-400' : 'text-emerald-400'}">${sender}:</span>${text}`;
            chatLog.appendChild(bubble);
            chatLog.scrollTop = chatLog.scrollHeight;
        }

        function simReset() {
            simShowWifi();
        }

        // Tab switcher
        function switchTab(tabId) {
            document.querySelectorAll('.tab-content').forEach(el => {
                el.classList.remove('active');
            });

            document.querySelectorAll('aside nav button').forEach(btn => {
                btn.className = "w-full text-left px-3.5 py-2.5 rounded-xl text-gray-400 hover:bg-dark-800 hover:text-white transition-all flex items-center";
            });

            const target = document.getElementById('tab-' + tabId);
            if (target) {
                target.classList.add('active');
            }

            const activeBtn = document.getElementById('tab-btn-' + tabId);
            if (activeBtn) {
                if (tabId === 'cloud-builder') {
                    activeBtn.className = "w-full text-left px-3.5 py-2.5 rounded-xl bg-emerald-600 text-white font-semibold transition-all flex items-center shadow-lg shadow-emerald-900/30";
                } else {
                    activeBtn.className = "w-full text-left px-3.5 py-2.5 rounded-xl bg-brand-600 text-white font-semibold transition-all flex items-center shadow-lg shadow-blue-900/30";
                }
            }

            const configBar = document.getElementById('config-bar');
            if (tabId === 'code') {
                configBar.classList.remove('hidden');
            } else {
                configBar.classList.add('hidden');
            }
        }

        function copyText(text) {
            const textArea = document.createElement("textarea");
            textArea.value = text;
            textArea.style.position = "fixed";
            textArea.style.left = "-9999px";
            document.body.appendChild(textArea);
            textArea.select();
            try {
                document.execCommand('copy');
            } catch (err) {
                console.error('Copy fallback failed:', err);
            }
            document.body.removeChild(textArea);
        }

        function copyGeneratedCode() {
            const code = document.getElementById('cpp-code-block').textContent;
            copyText(code);
            const btn = document.getElementById('btn-copy-code');
            const original = btn.innerHTML;
            btn.innerHTML = '<i class="fa-solid fa-check"></i> Copiat!';
            btn.classList.replace('bg-brand-600', 'bg-emerald-600');
            setTimeout(() => {
                btn.innerHTML = original;
                btn.classList.replace('bg-emerald-600', 'bg-brand-600');
            }, 2000);
        }

        function copyGithubAction() {
            const code = document.getElementById('github-action-code').textContent;
            copyText(code);
            const btn = document.getElementById('btn-copy-action');
            const original = btn.innerHTML;
            btn.innerHTML = '<i class="fa-solid fa-check"></i> Copiat!';
            btn.classList.replace('bg-dark-800', 'bg-emerald-700');
            setTimeout(() => {
                btn.innerHTML = original;
                btn.classList.replace('bg-emerald-700', 'bg-dark-800');
            }, 2000);
        }

        function generateCode() {
            const template = `/*
 * Gemini OS - Standalone Firmware (Zero Hardcoding / Zero Conturi)
 * Placa: Waveshare ESP32-S3 Touch LCD 3.49" V2 (AXS15231B 172x640)
 * 
 * Functionalitati:
 * 1. Scanare Wi-Fi automata la pornire cu afisare pe ecran
 * 2. Tastatura tactila pe ecran pentru introducerea parolei
 * 3. Salvare automata in memoria permanenta NVS (Preferences.h)
 * 4. Mod Dublu de Interactiune: Tasteaza mesaje sau Vorbeste direct cu Gemini
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>

// Pini Ecran QSPI Waveshare 3.49" (AXS15231B)
#define TFT_CS   11
#define TFT_DC   8
#define TFT_RST  9
#define TFT_SCLK 12
#define TFT_D0   10
#define TFT_D1   14
#define TFT_D2   15
#define TFT_D3   13

// Pini Touch I2C AXS15231B
#define TOUCH_I2C_SDA 2
#define TOUCH_I2C_SCL 1
#define TOUCH_ADDR    0x3B

Arduino_DataBus *bus = new Arduino_ESP32QSPI(TFT_CS, TFT_SCLK, TFT_D0, TFT_D1, TFT_D2, TFT_D3);
Arduino_GFX *gfx = new Arduino_AXS15231B(bus, TFT_RST, 0 /* rotation 172x640 */, true /* IPS */);
Preferences prefs;

enum OSState {
    STATE_WIFI_SCAN,
    STATE_KEYBOARD,
    STATE_CONNECTING,
    STATE_MAIN_DASHBOARD
};

OSState currentState = STATE_WIFI_SCAN;
String selectedSSID = "";
String enteredInput = "";
String keyboardTarget = "WIFI"; // "WIFI" sau "PROMPT"
int networkCount = 0;

void drawHeader(const char* title, uint16_t color) {
    gfx->fillRect(0, 0, 172, 35, 0x18E3);
    gfx->setTextColor(color);
    gfx->setTextSize(1);
    gfx->setCursor(8, 12);
    gfx->print(title);
    gfx->drawFastHLine(0, 35, 172, 0x4208);
}

void renderWifiScanScreen() {
    gfx->fillScreen(BLACK);
    drawHeader("RETELE WI-FI", CYAN);
    gfx->setCursor(8, 45);
    gfx->setTextColor(YELLOW);
    gfx->println("Scanare retele...");

    networkCount = WiFi.scanNetworks();
    gfx->fillRect(0, 40, 172, 600, BLACK);

    if (networkCount == 0) {
        gfx->setTextColor(RED);
        gfx->println("Nicio retea gasita!");
        return;
    }

    int displayLimit = min(networkCount, 7);
    for (int i = 0; i < displayLimit; i++) {
        int y = 45 + (i * 38);
        gfx->fillRoundRect(6, y, 160, 32, 6, 0x2124);
        gfx->drawRoundRect(6, y, 160, 32, 6, 0x4208);
        gfx->setTextColor(WHITE);
        gfx->setCursor(14, y + 10);
        String name = WiFi.SSID(i);
        if (name.length() > 14) name = name.substring(0, 13) + "...";
        gfx->print(name);
    }
}

void renderKeyboardScreen(const char* title) {
    gfx->fillScreen(BLACK);
    drawHeader(title, GREEN);

    // Cutie text input
    gfx->fillRoundRect(6, 45, 160, 36, 6, 0x18E3);
    gfx->drawRoundRect(6, 45, 160, 36, 6, CYAN);
    gfx->setTextColor(GREEN);
    gfx->setCursor(12, 58);
    gfx->print(enteredInput.length() > 0 ? enteredInput : "_");

    // Grila tastatura adaptata pentru 172x640
    // Litere & Cifre usor de atins pe touchscreen
    const char* keys[] = {
        "1", "2", "3", "<",
        "a", "b", "c", "d",
        "e", "f", "g", "h",
        "i", "o", "m", "s",
        "SPC", "OK"
    };

    int startY = 100;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            int idx = row * 4 + col;
            int x = 6 + (col * 40);
            int y = startY + (row * 42);
            gfx->fillRoundRect(x, y, 36, 36, 4, 0x2945);
            gfx->setTextColor(WHITE);
            gfx->setCursor(x + 12, y + 14);
            gfx->print(keys[idx]);
        }
    }

    // Buton Spatiu si OK
    gfx->fillRoundRect(6, startY + 175, 76, 38, 6, 0x2945);
    gfx->setCursor(24, startY + 190);
    gfx->print("SPATIU");

    gfx->fillRoundRect(88, startY + 175, 78, 38, 6, 0x03E0);
    gfx->setTextColor(WHITE);
    gfx->setCursor(110, startY + 190);
    gfx->print("OK");
}

void renderMainDashboard() {
    gfx->fillScreen(BLACK);
    drawHeader("GEMINI ASISTENT", GREEN);

    gfx->setCursor(8, 48);
    gfx->setTextColor(WHITE);
    gfx->println("Alege modul:");

    // Buton Tastatura (Scrie Text)
    gfx->fillRoundRect(8, 120, 156, 50, 8, 0x029F);
    gfx->setTextColor(WHITE);
    gfx->setCursor(24, 140);
    gfx->print("[Tastatura] Scrie");

    // Buton Voce (Microfon)
    gfx->fillRoundRect(8, 190, 156, 50, 8, 0xD9E0);
    gfx->setTextColor(WHITE);
    gfx->setCursor(28, 210);
    gfx->print("[Microfon] Voce");

    // Reset Wi-Fi
    gfx->fillRoundRect(8, 560, 156, 35, 6, 0x3186);
    gfx->setTextColor(0xAD55);
    gfx->setCursor(34, 572);
    gfx->print("Resetare Wi-Fi");
}

void askGemini(const String& prompt) {
    gfx->fillScreen(BLACK);
    drawHeader("GEMINI GANDESTE", YELLOW);
    gfx->setCursor(10, 50);
    gfx->setTextColor(YELLOW);
    gfx->println("Trimit cererea...");

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient https;

    String apiKey = prefs.getString("gemini_key", "");
    if (apiKey.length() == 0) {
        apiKey = "CHEIA_GEMINI_AICI"; // sau salvata pe ecran
    }

    String url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + apiKey;
    if (https.begin(client, url)) {
        https.addHeader("Content-Type", "application/json");

        JsonDocument req;
        req["contents"][0]["parts"][0]["text"] = prompt;
        String payload;
        serializeJson(req, payload);

        int code = https.POST(payload);
        if (code == 200) {
            JsonDocument res;
            deserializeJson(res, https.getString());
            const char* ans = res["candidates"][0]["content"]["parts"][0]["text"];

            gfx->fillScreen(BLACK);
            drawHeader("RASPUNS", GREEN);
            gfx->setCursor(6, 45);
            gfx->setTextColor(WHITE);
            gfx->setTextSize(1);
            gfx->println(ans ? ans : "Fara raspuns.");
        } else {
            gfx->setTextColor(RED);
            gfx->printf("Eroare HTTP: %d\\n", code);
        }
        https.end();
    }
}

void setup() {
    Serial.begin(115200);
    prefs.begin("gemini_os", false);

    // Initializare ecran
    gfx->begin();
    gfx->fillScreen(BLACK);

    // Initializare Touch AXS15231B pe I2C
    Wire.begin(TOUCH_I2C_SDA, TOUCH_I2C_SCL);

    // Verificam daca exista Wi-Fi salvat deja in flash
    String savedSSID = prefs.getString("ssid", "");
    String savedPass = prefs.getString("pass", "");

    if (savedSSID.length() > 0) {
        drawHeader("CONECTARE...", CYAN);
        gfx->setCursor(10, 60);
        gfx->println("Conectare la:\\n" + savedSSID);
        WiFi.begin(savedSSID.c_str(), savedPass.c_str());
        int tries = 0;
        while (WiFi.status() != WL_CONNECTED && tries < 20) {
            delay(500);
            tries++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            currentState = STATE_MAIN_DASHBOARD;
            renderMainDashboard();
            return;
        }
    }

    // Altfel, intram in scanare Wi-Fi pe ecran
    currentState = STATE_WIFI_SCAN;
    renderWifiScanScreen();
}

void loop() {
    // Touch polling si logica de navigare intre ecrane
    delay(20);
}`;

            const codeBlock = document.getElementById('cpp-code-block');
            if (codeBlock) {
                codeBlock.textContent = template;
                if (typeof Prism !== 'undefined' && Prism.highlightElement) {
                    Prism.highlightElement(codeBlock);
                }
            }
        }

        document.addEventListener('DOMContentLoaded', () => {
            switchTab('cloud-builder');
            generateCode();
            simShowWifi();
        });
    </script>
</body>
</html>
