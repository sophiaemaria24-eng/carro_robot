# 🤖 Robô 2WD com Controle PID, Odometria e Painel Web (ESP32)

Este repositório contém o código-fonte e a documentação de uma plataforma robótica móvel 2WD desenvolvida com **ESP32-C3 Super Mini**, driver de motores **L298N** e **encoders ópticos**. O robô realiza deslocamentos lineares entre **0 e 1 metro (100 cm)**, permitindo testar e comparar o comportamento em **Malha Aberta** e em **Malha Fechada** com os controladores clássicos **P, I, D, PI, PD e PID**, tudo operado em tempo real por uma Interface Homem-Máquina (IHM) via navegador no celular ou computador.

> **Guia para iniciantes:** Este README foi escrito passo a passo para quem nunca utilizou o **PlatformIO** no **VS Code** nem gravou códigos em um **ESP32**.

---

## 📋 Sumário

1. [Visão Geral do Projeto](#1-visão-geral-do-projeto)
2. [Como Instalar o VS Code e o PlatformIO](#2-como-instalar-o-vs-code-e-o-platformio)
3. [Entendendo as Pastas do PlatformIO](#3-entendendo-as-pastas-do-platformio)
4. [Entendendo os Botões (Funções) do PlatformIO](#4-entendendo-os-botões-funções-do-platformio)
5. [Como Enviar (Gravar) o Código no ESP32](#5-como-enviar-gravar-o-código-no-esp32)
6. [Como Acessar e Usar o Painel de Controle (IHM)](#6-como-acessar-e-usar-o-painel-de-controle-ihm)
7. [Como o Código Funciona (Resumo Simplificado)](#7-como-o-código-funciona-resumo-simplificado)
8. [Mapa de Conexões (Pinagem)](#8-mapa-de-conexões-pinagem)
9. [Solução de Problemas Comuns](#9-solução-de-problemas-comuns)

---

## 1. Visão Geral do Projeto

O sistema funciona de forma totalmente autônoma e sem precisar de roteador ou acesso à internet:

* **Microcontrolador:** ESP32-C3 Super Mini.
* **Atuação:** Driver Ponte H L298N controlando 2 motores DC com caixa de redução (tipo TT).
* **Sensoriamento (Odometria):** 2 discos perfurados acoplados aos eixos das rodas + 2 sensores ópticos (alimentados em 3,3 V).
* **Interface (IHM):** O próprio ESP32 cria uma rede Wi-Fi e hospeda uma página Web completa diretamente na sua memória (embutida no `main.cpp`), permitindo ajustar o alvo (*Setpoint*), os ganhos ($K_p$, $K_i$, $K_d$) e monitorar a posição, o erro e a velocidade (PWM) em tempo real.

---

## 2. Como Instalar o VS Code e o PlatformIO

O **PlatformIO** é uma extensão gratuita dentro do **Visual Studio Code (VS Code)** que compila o código C++ e o envia para placas como o ESP32 através do cabo USB.

### Passo 1: Instalar o Visual Studio Code
1. Acesse [code.visualstudio.com](https://code.visualstudio.com/) e faça o download para o seu sistema operacional (Windows, macOS ou Linux).
2. Execute o instalador e clique em **Avançar** até concluir.

### Passo 2: Instalar a Extensão PlatformIO IDE
1. Abra o **VS Code**.
2. Na barra lateral esquerda, clique no ícone de **Extensões** (quatro quadradinhos, ou pressione `Ctrl + Shift + X`).
3. Na barra de pesquisa, digite **`PlatformIO IDE`**.
4. Clique no botão azul **Install (Instalar)** na extensão oficial da *PlatformIO*.
5. **Aguarde a instalação terminar:** no canto inferior direito aparecerá uma barra de progresso. Pode levar de 2 a 5 minutos, pois ele baixa automaticamente o compilador e o ambiente Python interno.
6. Quando aparecer a mensagem pedindo para recarregar (**Reload Now**), clique nela (ou feche e abra o VS Code novamente).
7. Um ícone de **formiga/alienígena** aparecerá na barra lateral esquerda — isso confirma que o PlatformIO está pronto.

### Passo 3: Abrir este Projeto no VS Code
1. Baixe este repositório (em **Code > Download ZIP** no GitHub e extraia a pasta, ou use `git clone`).
2. No VS Code, vá em **File > Open Folder...** (Arquivo > Abrir Pasta).
3. Selecione a pasta principal do projeto (a pasta que contém o arquivo `platformio.ini`).
4. Aguarde alguns segundos até o PlatformIO carregar as dependências automaticamente.

---

## 3. Entendendo as Pastas do PlatformIO

Ao abrir o projeto no VS Code, você verá a seguinte estrutura de pastas:

```text
ProjetoRobo/
├── .pio/              # Arquivos gerados automaticamente na compilação (não precisa mexer)
├── .vscode/           # Configurações internas do VS Code (não precisa mexer)
├── include/           # Pasta para arquivos de cabeçalho (.h) em projetos maiores
├── lib/               # Pasta para bibliotecas externas adicionadas manualmente
├── src/               # ⭐ PASTA PRINCIPAL: onde fica o código do robô
│   └── main.cpp       # ⭐ Arquivo principal com toda a lógica PID, Wi-Fi e página Web
├── test/              # Pasta destinada a testes automatizados (não usada aqui)
├── .gitignore         # Define quais arquivos temporários o GitHub deve ignorar
└── platformio.ini     # ⭐ Arquivo de configuração da placa (ESP32-C3) e da porta USB
```

### Os 2 arquivos que realmente importam para você:
* **`platformio.ini`**: Diz ao PlatformIO qual é o modelo exato da placa (`esp32-c3-devkitm-1`), qual framework usar (`arduino`) e qual a velocidade da comunicação serial (`115200`).
* **`src/main.cpp`**: Contém **todo o cérebro do projeto** — configuração dos pinos, leitura dos encoders, cálculo do controlador PID, criação da rede Wi-Fi e o código da página Web (HTML, CSS e JavaScript).

---

## 4. Entendendo os Botões (Funções) do PlatformIO

Na **barra azul na parte inferior** do VS Code, o PlatformIO adiciona atalhos essenciais para trabalhar com o ESP32:

| Ícone | Nome | O que faz na prática |
| :---: | :--- | :--- |
| 🏠 | **PlatformIO Home** | Abre a tela inicial do PlatformIO para criar projetos, buscar bibliotecas ou ver portas USB conectadas. |
| ✔️ | **Build (Compilar)** | Traduz o código C++ para a linguagem da placa e verifica se existe algum erro de digitação, **sem** enviar para o ESP32. |
| ➡️ | **Upload (Enviar)** | Compila o código e **grava (transfere)** o programa na memória do ESP32 pelo cabo USB. |
| 🗑️ | **Clean (Limpar)** | Apaga arquivos temporários de compilações antigas (útil se o compilador apresentar algum erro estranho). |
| 🔌 | **Serial Monitor** | Abre um terminal na tela para ler mensagens de texto que o ESP32 envia de volta ao computador (como o status do servidor). |

---

## 5. Como Enviar (Gravar) o Código no ESP32

Como toda a interface Web já está embutida dentro do arquivo `src/main.cpp`, **você só precisa fazer um único envio**:

1. Conecte o **ESP32-C3** ao computador usando um **cabo USB-C de dados** (cabos que servem apenas para carregar celular não funcionam).
2. Abra o projeto no VS Code.
3. Clique no ícone da **Seta para a direita (`➡️` — PlatformIO: Upload)** na barra inferior azul.
4. Um terminal abrirá na parte de baixo mostrando a compilação e o envio (`Writing at 0x00010000... 100%`).
5. Quando aparecer a mensagem verde **`[SUCCESS]`**, o código já está gravado e rodando dentro do ESP32!
6. *(Opcional)* Clique no ícone de **Tomada (`🔌` — Serial Monitor)** para acompanhar as mensagens do ESP32. Lembre-se de fechar o Monitor Serial (clicando no ícone de lixeira do terminal) antes de fazer um novo Upload.

> **Dica:** Na primeira vez que você compilar o projeto em um computador novo, o PlatformIO precisará de internet para baixar o pacote da placa ESP32. Depois disso, funciona totalmente offline.

---

## 6. Como Acessar e Usar o Painel de Controle (IHM)

Assim que o ESP32 for ligado (pelo USB ou pela bateria do robô), ele cria automaticamente a própria rede Wi-Fi:

1. No seu **celular ou notebook**, abra as configurações de **Wi-Fi**.
2. Conecte-se à rede:
   * **Nome da rede (SSID):** `Robo_2WD_Avaliacao`
   * **Senha:** `12345678`
   * *(O aparelho pode avisar "Conectado sem internet" — isso é normal, mantenha a conexão).*
3. Abra qualquer navegador (Chrome, Edge, Safari) e digite na barra de endereços:
   ```text
   [http://192.168.4.1](http://192.168.4.1)
   ```
4. O **Painel de Controle PID** será exibido na tela.

### O que você pode fazer no painel:
* **Modo de Controle:** Alternar entre **Malha Aberta** e **Malha Fechada** (escolhendo **P, I, D, PI, PD ou PID**).
* **Setpoint (Alvo):** Definir a distância que o robô deve percorrer, de **0,0 cm até 100,0 cm (1 metro)**.
* **Ganhos ($K_p$, $K_i$, $K_d$):** Ajustar os parâmetros do controlador usando os controles deslizantes (*sliders*).
* **Comandos:**
  * **▶ Iniciar Experimento:** Envia as configurações escolhidas e inicia o movimento do robô.
  * **✋ Parar Robô:** Interrompe imediatamente os motores.
  * **🔄 Reiniciar Odometria:** Zera a contagem dos encoders, a posição atual e os cálculos acumulados do PID para um novo teste.
* **Monitoramento em Tempo Real:** Exibe continuamente o **Status**, a **Posição Atual (cm)**, o **Erro (cm)** e a **Velocidade aplicada aos motores (PWM de 0 a 255)**.

---

## 7. Como o Código Funciona (Resumo Simplificado)

O arquivo `src/main.cpp` é dividido em 4 pilares que trabalham juntos:

### 1. Leitura dos Encoders (Odometria)
Os sensores ópticos estão ligados a **interrupções de hardware** (`IRAM_ATTR`). Sempre que um furo do disco passa pelo feixe de luz do sensor, o ESP32 pausa qualquer tarefa por microssegundos e soma `+1` no contador daquela roda (`pulsosEsq` ou `pulsosDir`), garantindo que nenhum pulso seja perdido.

A distância percorrida é estimada pela média das duas rodas:

$$\text{Distância (cm)} = \left(\frac{\text{pulsosEsq} + \text{pulsosDir}}{2}\right) \times \frac{\pi \times D_{\text{roda}}}{\text{Pulsos por Volta}}$$

*(Caso o seu disco tenha um número diferente de furos ou a roda tenha outro diâmetro, basta ajustar as constantes `DIAMETRO_RODA_CM` e `PULSOS_POR_VOLTA` no início do `main.cpp`).*

### 2. Rede Wi-Fi Própria e Comunicação HTTP
* O ESP32 inicia em modo **Access Point (`WIFI_AP`)** com IP fixo `192.168.4.1` e potência de transmissão reduzida (`WIFI_POWER_5dBm`) para manter a conexão estável em curta distância sem interferências.
* A página Web (HTML + CSS + JS) fica guardada na memória Flash (`PROGMEM`) e conversa com o robô através de 3 rotas HTTP:
  * `GET /` → Entrega a página visual do Painel de Controle para o navegador.
  * `POST /comando` → Recebe do navegador o botão clicado (`iniciar`, `parar`, `reiniciar`), o modo selecionado, o Setpoint e os ganhos $K_p, K_i, K_d$.
  * `GET /dados` → Chamado automaticamente pelo navegador a cada **300 ms**; o ESP32 devolve um JSON com posição, erro, PWM e status para atualizar a tela ao vivo.

### 3. Algoritmo de Controle (Loop de 50 ms)
Enquanto o experimento está ativo, a cada **50 milissegundos** (`dt >= 0.05 s`) o ESP32 executa:
1. Atualiza a `distanciaAtual` com base nos encoders.
2. Calcula o erro instantâneo: $\text{erro} = \text{setpoint} - \text{distanciaAtual}$.
3. **Em Malha Aberta:** aciona os motores com PWM fixo (`150`) até atingir o alvo.
4. **Em Malha Fechada:** calcula as parcelas **Proporcional ($P$)**, **Integral ($I$)** e **Derivativa ($D$)**, ativa apenas os termos correspondentes ao modo selecionado (`P`, `I`, `D`, `PI`, `PD` ou `PID`) e soma o sinal de controle.
5. **Zona Morta (Banda Morta):** se o erro for menor que `1.0 cm`, o sinal é zerado para evitar que o robô fique vibrando no ponto final.

### 4. Acionamento dos Motores (PWM)
A função `acionarMotores(int pwm)` define o sentido de rotação (frente se positivo, ré se negativo, parado se zero) pelos pinos `IN1`–`IN4` e regula a força/velocidade de `0` a `255` via sinal **PWM** nos pinos `ENA` e `ENB` do driver L298N.

---

## 8. Mapa de Conexões (Pinagem)

| Componente | Pino do Componente | Pino no ESP32-C3 Super Mini | Função |
| :--- | :---: | :---: | :--- |
| **L298N (Motor Esquerdo)** | `IN1` | `GPIO 8` | Sentido de giro (Frente) |
| **L298N (Motor Esquerdo)** | `IN2` | `GPIO 9` | Sentido de giro (Ré) |
| **L298N (Motor Esquerdo)** | `ENA` | `GPIO 10` | Controle de velocidade (PWM) |
| **L298N (Motor Direito)** | `IN3` | `GPIO 6` | Sentido de giro (Frente) |
| **L298N (Motor Direito)** | `IN4` | `GPIO 7` | Sentido de giro (Ré) |
| **L298N (Motor Direito)** | `ENB` | `GPIO 5` | Controle de velocidade (PWM) |
| **Encoder Esquerdo** | `D0` / `OUT` | `GPIO 3` | Leitura dos pulsos (3,3 V) |
| **Encoder Direito** | `D0` / `OUT` | `GPIO 4` | Leitura dos pulsos (3,3 V) |
| **Encoders (Alimentação)** | `VCC` e `GND` | `3V3` e `GND` | Alimentação segura em 3,3 V |
| **Referência Comum** | `GND` (L298N / Baterias) | `GND` | **GND comum obrigatório** |

> ⚠️ **Atenção elétrica:**
> * Alimente os sensores ópticos **sempre em `3V3`** (nunca em `5V`) para proteger os pinos do ESP32.
> * Interligue obrigatoriamente o **`GND` do ESP32** ao borne **`GND` do L298N**.
> * Retire os jumpers pretos dos pinos **`ENA`** e **`ENB`** do L298N para que o controle de velocidade por PWM funcione.

---

## 9. Solução de Problemas Comuns

* **Erro `PermissionError / Acesso negado` na porta COM ao fazer Upload:**
  * Outro programa (como o próprio Monitor Serial do VS Code, Arduino IDE ou Thonny) está usando a porta USB. Clique na lixeira do terminal do VS Code para fechar o Monitor Serial, desconecte e reconecte o cabo USB e tente clicar em **Upload** novamente.
* **O Upload trava na mensagem `Connecting...`:**
  * Segure o botão **`BOOT`** na placa do ESP32 por cerca de 2 segundos logo após clicar em Upload, depois solte.
* **A rede Wi-Fi `Robo_2WD_Avaliacao` não aparece:**
  * Verifique se o ESP32 está ligado (LED aceso) e aproxime o celular/notebook do robô, pois o sinal está intencionalmente configurado para curto alcance (`5 dBm`).
* **O robô anda mais ou menos do que a distância configurada:**
  * Meça o diâmetro real da roda e conte quantos pulsos o disco gera em exatamente 1 volta completa; depois ajuste `DIAMETRO_RODA_CM` e `PULSOS_POR_VOLTA` em `src/main.cpp` e faça o Upload novamente.
