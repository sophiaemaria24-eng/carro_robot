let modoAtual = "Malha Aberta";

const sliders = ['setpoint', 'kp', 'ki', 'kd'];
sliders.forEach(id => {
    const el = document.getElementById(id);
    const valEl = document.getElementById(id === 'setpoint' ? 'valSet' : 'val' + id.charAt(0).toUpperCase() + id.slice(1));
    el.addEventListener('input', (e) => {
        valEl.innerText = parseFloat(e.target.value).toFixed(id === 'setpoint' ? 1 : 2);
    });
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
    fetch('/dados')
    .then(res => res.json())
    .then(data => {
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