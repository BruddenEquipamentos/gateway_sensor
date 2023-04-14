# gateway_sensor
Gateway help button Next

Necessário: Visual Studio Code + PlatfomIO (extensão)
Esquemático de conexões: Verificar com o Mike.
O endereço de escuta do Gateway corresponde "2Node".
É necessário criar a partição no ESP32 e gravar o arquivo em branco address.txt (/data), apenas uma vez.

Funcionamento:
Com o sensoriamento + ble (necessário display já configurado - pois associasse com NSU do equipamento), através do aplicativo, ao configurar o pedido de ajuda, o rádio do sensoriamento envia um pacote ao gateway que corresponde:
DataOut[0] = '2' -> Solicitação para configurar
DataOut[1] = ID AJuda (1~255) Máquina
DataOut[2] = NSU
DataOut[3] = NSU
DataOut[4] = NSU
DataOut[5] = NSU

O gateway por sua vez salva no arquivo (persistir os dados caso ocorra algum problema) e atualiza na memória uma matriz na qual corresponde:
Linha 0 (Id 1) NSU  NSU NSU NSU
Linha 1 (Id 2) 0    0   0   0
...
Linha 254 (Id 255) 0  0 0 0

Quando o botão de ajuda é acionado, o rádio do display envia para o gateway um pacote com o respectivo NSU registrado (visto que esse NSU é salvo quando configurado o display).
De tal forma que o gateway busca esse NSU recebido na matriz, ao encontrar a linha correspondente, ele identifica o ID da máquina;

Rádio do Display:
DataOut[0] = '1' -> Solicitação de ajuda
DataOut[1] = NSU
DataOut[2] = NSU
DataOut[3] = NSU
DataOut[4] = NSU

Rádio do Display:
DataOut[0] = '0' -> Solicitação de cancelamento de ajuda
DataOut[1] = NSU
DataOut[2] = NSU
DataOut[3] = NSU
DataOut[4] = NSU

O que falta ser feito:
Ao chegar o relógio é necessário integrar pela porta serial do Gateway a antena que enviará os comandos para o relógio. Verificar protocolo anexo pelo fornecedor.
