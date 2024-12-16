# Prática #10 (NodeRed)

Apresentar um aplicativo em Node-Red que receba do NodeMCU as seguintes informações:

1. Temperatura  - Gráfico
2. Umidade - Nível
3. A tensão de um de dois fotoresistores, apresentado como iluminado, meia luz ou apagado.

A partir do painel deverá selecionar as unidades em que esses valores serão apresentados da seguinte maneira:

1. Temperatura: graus Celsius ou Fahrenheit
2. Umidade: (kg/m³),  (lb/ft³), ou relativa (100%). 
3. Escolher qual o fotoresistor que vai ser monitorado.

No gráfico de temperatura, apresentar também a média móvel dos últimos 7 dados. 

Ativar um alarme luminoso e sonoro no painel se a temperatura passar de um determinado valor, piscar um led de alarme no NodeMCU enquanto durar esta condição. Além disso, enviar uma mensagem via twitter/telegram caso a temperatura/humidade passem de um determinado valor.
A partir do painel, com um slider, definir a posição de  um motor de passo conectado ao NodeMCU, e receber uma mensagem via twitter/telegram e acionar o motor de passo remotamente também.
Instalar o broker no próprio computador que executa o NodeRed com o módulo Aedes.