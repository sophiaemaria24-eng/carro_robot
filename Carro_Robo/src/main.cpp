#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// ==========================================
// 1. PINAGEM DO L298N E ENCODERS (ESP32-C3)
// ==========================================
const int IN1 = 8;  
const int IN2 = 9;  
const int ENA = 10; 

const int IN3 = 6;  
const int IN4 = 7;  
const int ENB = 5;  

const int ENC_ESQ = 3; 
const int ENC_DIR = 4; 

// ==========================================
// 2. VARIÁVEIS DE ODOMETRIA E PID
// ==========================================
volatile long pulsosEsq = 0;
volatile long pulsosDir = 0;

const float DIAMETRO_RODA_CM = 6.6; 
const float PULSOS_POR_VOLTA = 20.0; 
const float CIRCUNFERENCIA_CM = PI * DIAMETRO_RODA_CM;
const float CM_POR_PULSO = CIRCUNFERENCIA_CM / PULSOS_POR_VOLTA;

float Kp = 0.0, Ki = 0.0, Kd = 0.0;
float setpoint_cm = 0.0;
String modoControle = "Malha Aberta";

float erro = 0, erroAnterior = 0, integral = 0, derivada = 0;
float distanciaAtual = 0;
int velocidadeComandada = 0;
bool rodando = false;
unsigned long tempoAnterior = 0;

const int canalEsq = 0;
const int canalDir = 1;

void IRAM_ATTR isrEncoderEsq() { pulsosEsq++; }
void IRAM_ATTR isrEncoderDir() { pulsosDir++; }

WebServer server(80);

// ==========================================
// 3. PÁGINA WEB EMBUTIDA (IHM Completa)
// ==========================================
const char html_IHM[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Painel de Controle PID</title>
    <style>
        :root { --bg-color: #f4f6f9; --card-bg: #ffffff; --primary: #311b58; --text-main: #2d3748; --border-color: #e2e8f0; --green: #059669; --red: #dc2626; --yellow-bg: #fef08a; --yellow-text: #854d0e; }
        body { font-family: 'Segoe UI', Tahoma, sans-serif; margin: 0; padding: 30px; background: var(--bg-color); color: var(--text-main); }
        header h1 { margin: 0; font-size: 26px; }
        header p { margin: 5px 0 25px 0; color: #718096; }
        .grid-layout { display: grid; grid-template-columns: 1.2fr 1fr; gap: 20px; max-width: 1100px; }
        .card { background: var(--card-bg); padding: 25px; border-radius: 12px; box-shadow: 0 4px 15px rgba(0,0,0,0.03); margin-bottom: 20px; }
        .section-title { font-size: 11px; text-transform: uppercase; letter-spacing: 1px; color: #718096; margin-top: 0; margin-bottom: 20px; }
        .sub-section-title { font-size: 11px; text-transform: uppercase; letter-spacing: 1px; color: #718096; margin: 25px 0 15px 0; }
        .control-group { margin-bottom: 20px; }
        label { font-weight: bold; font-size: 12px; display: block; margin-bottom: 8px; }
        .flex-between { display: flex; justify-content: space-between; align-items: center; }
        .mb-20 { margin-bottom: 20px; } .mt-20 { margin-top: 20px; }
        .mode-toggle { display: flex; background: #f1f5f9; border-radius: 8px; overflow: hidden; }
        .toggle-btn { flex: 1; padding: 12px; border: none; background: transparent; font-weight: bold; color: #718096; cursor: pointer; transition: 0.3s; }
        .toggle-btn.active { background: var(--primary); color: white; }
        .modern-select { width: 100%; padding: 10px; border-radius: 8px; border: 1px solid var(--border-color); font-size: 14px; }
        .box-outline { background: #fafafc; border: 1px solid var(--border-color); padding: 15px; border-radius: 12px; }
        .value-pill { background: #fff; border: 1px solid var(--border-color); padding: 4px 12px; border-radius: 20px; font-weight: bold; }
        .val-bold { font-weight: bold; font-size: 15px; }
        input[type=range] { width: 100%; margin: 15px 0 5px 0; accent-color: var(--primary); }
        .slider-labels { font-size: 10px; color: #718096; }
        .gains-grid { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 10px; }
        .status-pill { background: var(--yellow-bg); color: var(--yellow-text); padding: 8px 15px; border-radius: 20px; font-size: 12px; display: flex; align-items: center; gap: 8px; }
        .status-dot { width: 8px; height: 8px; background: #eab308; border-radius: 50%; }
        .monitor-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; }
        .monitor-value { font-size: 32px; font-weight: bold; margin-top: 10px; }
        .unit { font-size: 14px; font-weight: normal; color: #718096; }
        .btn-group { display: grid; grid-template-columns: 2fr 2fr 1.5fr; gap: 10px; }
        .btn { padding: 15px; border: none; border-radius: 8px; font-size: 13px; font-weight: bold; cursor: pointer; color: white; }
        .btn-start { background: var(--green); } .btn-stop { background: var(--red); }
        .btn-reset { background: #fff; color: var(--text-main); border: 1px solid var(--border-color); }
        @media (max-width: 768px) { .grid-layout { grid-template-columns: 1fr; } .btn-group, .gains-grid { grid-template-columns: 1fr; } }
    </style>
</head>
<body>
    <header><h1>Painel de Controle PID</h1><p>Sistema de monitoramento e controle de posição</p></header>
    <div class="grid-layout">
        <section class="coluna-esq">
            <div class="card">
                <h3 class="section-title">CONFIGURAÇÃO PID</h3>
                <div class="control-group">
                    <label>MODO DE CONTROLE</label>
                    <div class="mode-toggle">
                        <button id="btn-aberta" class="toggle-btn active" onclick="setMode('Malha Aberta')">Malha Aberta</button>
                        <button id="btn-fechada" class="toggle-btn" onclick="setMode('Malha Fechada')">Malha Fechada</button>
                    </div>
                </div>
                <div class="control-group" id="tipo-controle-group" style="display: none;">
                    <label>TIPO DE CONTROLADOR</label>
                    <select id="tipo-pid" class="modern-select">
                        <option value="P">P</option><option value="I">I</option><option value="D">D</option>
                        <option value="PI">PI</option><option value="PD">PD</option><option value="PID" selected>PID</option>
                    </select>
                </div>
                <div class="control-group box-outline">
                    <div class="flex-between">
                        <label>Setpoint (Alvo)<br><small style="color:#718096; font-weight:normal;">Posição linear de referência</small></label>
                        <div class="value-pill"><span id="valSet">0.0</span> cm</div>
                    </div>
                    <input type="range" id="setpoint" min="0" max="100" step="0.5" value="0">
                    <div class="flex-between slider-labels"><span>0.0 cm</span><span>Faixa de Operação: 0 - 100 cm</span><span>100.0 cm</span></div>
                </div>
                <h4 class="sub-section-title">GANHOS DO CONTROLADOR</h4>
                <div class="gains-grid">
                    <div class="gain-box box-outline">
                        <div class="flex-between"><label>Ganho Kp</label><span id="valKp" class="val-bold">0.00</span></div>
                        <input type="range" id="kp" min="0" max="10" step="0.01" value="0">
                        <div class="slider-labels text-right"><span>Máx: 10.0</span></div>
                    </div>
                    <div class="gain-box box-outline">
                        <div class="flex-between"><label>Ganho Ki</label><span id="valKi" class="val-bold">0.00</span></div>
                        <input type="range" id="ki" min="0" max="5" step="0.01" value="0">
                        <div class="slider-labels text-right"><span>Máx: 5.00</span></div>
                    </div>
                    <div class="gain-box box-outline">
                        <div class="flex-between"><label>Ganho Kd</label><span id="valKd" class="val-bold">0.00</span></div>
                        <input type="range" id="kd" min="0" max="5" step="0.01" value="0">
                        <div class="slider-labels text-right"><span>Máx: 5.00</span></div>
                    </div>
                </div>
            </div>
            <div class="card command-section">
                <h3 class="section-title">COMANDOS</h3>
                <div class="btn-group">
                    <button class="btn btn-start" onclick="enviarComando('iniciar')">▶ Iniciar Experimento</button>
                    <button class="btn btn-stop" onclick="enviarComando('parar')">✋ Parar Robô</button>
                    <button class="btn btn-reset" onclick="enviarComando('reiniciar')">🔄 Reiniciar Odometria</button>
                </div>
            </div>
        </section>
        <section class="coluna-dir">
            <div class="card">
                <div class="flex-between align-center mb-20">
                    <h3 class="section-title m-0">Monitoramento em<br>Tempo Real</h3>
                    <div class="status-pill" id="status-pill"><span class="status-dot"></span> Status:<br><strong id="statusRobo">Aguardando...</strong></div>
                </div>
                <div class="monitor-grid">
                    <div class="monitor-box box-outline"><label>POSIÇÃO ATUAL</label><div class="monitor-value"><span id="posicao">0.00</span> <span class="unit">cm</span></div></div>
                    <div class="monitor-box box-outline"><label>ERRO</label><div class="monitor-value"><span id="erro">0.00</span> <span class="unit">cm</span></div></div>
                </div>
                <div class="box-outline mt-20">
                    <div class="flex-between"><label>VELOCIDADE (PWM)</label><span id="vel" class="val-bold text-xl">0</span></div>
                    <input type="range" disabled min="0" max="255" value="0" id="vel-bar" style="accent-color: #718096;">
                    <div class="flex-between slider-labels"><span>0 (Repouso)</span><span>Intensidade Atuador</span><span>255 (Máx)</span></div>
                </div>
            </div>
        </section>
    </div>
    <script>
        let modoAtual = "Malha Aberta";
        const sliders = ['setpoint', 'kp', 'ki', 'kd'];
        sliders.forEach(id => {
            const el = document.getElementById(id);
            const valEl = document.getElementById(id === 'setpoint' ? 'valSet' : 'val' + id.charAt(0).toUpperCase() + id.slice(1));
            el.addEventListener('input', (e) => { valEl.innerText = parseFloat(e.target.value).toFixed(id === 'setpoint' ? 1 : 2); });
        });
        function setMode(mode) {
            modoAtual = mode;
            document.getElementById('btn-aberta').classList.toggle('active', mode === 'Malha Aberta');
            document.getElementById('btn-fechada').classList.toggle('active', mode === 'Malha Fechada');
            document.getElementById('tipo-controle-group').style.display = mode === 'Malha Fechada' ? 'block' : 'none';
        }
        function enviarComando(acao) {
            const sp = document.getElementById('setpoint').value;
            const kp = document.getElementById('kp').value;
            const ki = document.getElementById('ki').value;
            const kd = document.getElementById('kd').value;
            const modoFinal = modoAtual === 'Malha Fechada' ? document.getElementById('tipo-pid').value : 'Malha Aberta';
            const dados = `acao=${acao}&modo=${modoFinal}&sp=${sp}&kp=${kp}&ki=${ki}&kd=${kd}`;
            fetch('/comando', { method: 'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, body: dados });
        }
        setInterval(() => {
            fetch('/dados').then(res => res.json()).then(data => {
                document.getElementById('posicao').innerText = data.posicao.toFixed(2);
                document.getElementById('erro').innerText = data.erro.toFixed(2);
                document.getElementById('vel').innerText = data.velocidade;
                document.getElementById('vel-bar').value = data.velocidade;
                const statusEl = document.getElementById('statusRobo');
                const statusPill = document.getElementById('status-pill');
                const dot = statusPill.querySelector('.status-dot');
                if (data.rodando) {
                    statusEl.innerText = "Em movimento";
                    statusPill.style.backgroundColor = "#dcfce7"; statusPill.style.color = "#166534"; dot.style.backgroundColor = "#22c55e";
                } else {
                    statusEl.innerText = "Parado";
                    statusPill.style.backgroundColor = "#fef08a"; statusPill.style.color = "#854d0e"; dot.style.backgroundColor = "#eab308";
                }
            });
        }, 300);
    </script>
</body>
</html>
)rawliteral";

// ==========================================
// 4. ROTAS DO SERVIDOR WEB
// ==========================================
void handleRoot() {
    server.send(200, "text/html", html_IHM);
}

void handleComandos() {
    if (server.hasArg("acao")) {
        String acao = server.arg("acao");
        modoControle = server.arg("modo");
        setpoint_cm = server.arg("sp").toFloat();
        Kp = server.arg("kp").toFloat();
        Ki = server.arg("ki").toFloat();
        Kd = server.arg("kd").toFloat();

        if (acao == "iniciar") {
            rodando = true;
            tempoAnterior = millis();
        } else if (acao == "parar") {
            rodando = false;
            velocidadeComandada = 0;
            ledcWrite(canalEsq, 0);
            ledcWrite(canalDir, 0);
        } else if (acao == "reiniciar") {
            rodando = false;
            pulsosEsq = 0;
            pulsosDir = 0;
            integral = 0;
            erroAnterior = 0;
            velocidadeComandada = 0;
            ledcWrite(canalEsq, 0);
            ledcWrite(canalDir, 0);
        }
        server.send(200, "text/plain", "OK");
    }
}

void handleDados() {
    String json = "{";
    json += "\"posicao\":" + String(distanciaAtual) + ",";
    json += "\"erro\":" + String(erro) + ",";
    json += "\"velocidade\":" + String(velocidadeComandada) + ",";
    json += "\"rodando\":" + String(rodando ? "true" : "false") + "}";
    server.send(200, "application/json", json);
}

// ==========================================
// 5. ATUAÇÃO DOS MOTORES
// ==========================================
void acionarMotores(int pwm) {
    if (pwm > 0) { // Frente
        digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    } else if (pwm < 0) { // Trás
        digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    } else { // Parado
        digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    }
    int pwmLimitado = constrain(abs(pwm), 0, 255);
    ledcWrite(canalEsq, pwmLimitado);
    ledcWrite(canalDir, pwmLimitado);
    velocidadeComandada = (pwm == 0) ? 0 : pwmLimitado;
}

// ==========================================
// 6. SETUP
// ==========================================
void setup() {
    Serial.begin(115200);

    // Configuração de Pinos e PWM
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
    ledcSetup(canalEsq, 1000, 8); ledcAttachPin(ENA, canalEsq);
    ledcSetup(canalDir, 1000, 8); ledcAttachPin(ENB, canalDir);
    acionarMotores(0);

    // Configuração dos Encoders
    pinMode(ENC_ESQ, INPUT_PULLUP);
    pinMode(ENC_DIR, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENC_ESQ), isrEncoderEsq, RISING);
    attachInterrupt(digitalPinToInterrupt(ENC_DIR), isrEncoderDir, RISING);

    // Iniciar Wi-Fi AP (Rede Própria)
    WiFi.mode(WIFI_AP);
    IPAddress ip(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.softAP("Robo_2WD_Avaliacao", "12345678");

    // Reduzir Sinal Wi-Fi
    WiFi.setTxPower(WIFI_POWER_5dBm);

    // Configurar Servidor Web com a string HTML acima
    server.on("/", handleRoot);
    server.on("/comando", HTTP_POST, handleComandos);
    server.on("/dados", HTTP_GET, handleDados);
    server.begin();
    
    Serial.println("Servidor Pronto. Conecte no Wi-Fi 'Robo_2WD_Avaliacao' e acesse 192.168.4.1");
}

// ==========================================
// 7. LOOP E ALGORITMO PID
// ==========================================
void loop() {
    server.handleClient();

    if (rodando) {
        unsigned long tempoAtual = millis();
        float dt = (tempoAtual - tempoAnterior) / 1000.0; 

        if (dt >= 0.05) { // Controle de 50ms
            long mediaPulsos = (pulsosEsq + pulsosDir) / 2;
            distanciaAtual = mediaPulsos * CM_POR_PULSO;
            erro = setpoint_cm - distanciaAtual;

            if (modoControle == "Malha Aberta") {
                if (erro > 0) acionarMotores(150);
                else acionarMotores(0);
            } else {
                float P = Kp * erro;
                integral += Ki * erro * dt;
                derivada = Kd * (erro - erroAnterior) / dt;
                
                if (modoControle == "P") { integral = 0; derivada = 0; }
                else if (modoControle == "I") { P = 0; derivada = 0; }
                else if (modoControle == "D") { P = 0; integral = 0; }
                else if (modoControle == "PI") { derivada = 0; }
                else if (modoControle == "PD") { integral = 0; }
                
                float sinalControle = P + integral + derivada;
                if (abs(erro) < 1.0) sinalControle = 0; 
                
                acionarMotores((int)sinalControle);
                erroAnterior = erro;
            }
            tempoAnterior = tempoAtual;
        }
    }
}