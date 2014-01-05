porteiro
========

Repositório do porteiro do Tarrafa

O que é?
--------

O porteiro é o programa que roda no mula-sem-cabeça e serve principalmente para avisar quando tem ou não gente no Tarrafa. O porteiro centraliza várias funções. São elas: botão de presença, painel de LEDs, tomadas controladas por relé (pra ninguém esquecer a cafeteira ligada) e 'mood lamp'.

Estrutura dos arquivos
----------------------

A pasta 'arduino' tem o código que roda na plaquinha de arduino do porteiro. Este código atualiza o painel de LEDs com informação vinda pela serial, manda informação de alteração do estado do botão pela serial e controla a tomada e o 'mood lamp'.

A pasta 'eagle' tem os esquemáticos e o layout da plaquinha do shield de arduino do porteiro.

A pasta 'fonte' contém a fonte criada colaborativamente pelo Tarrafa para mostrar coisas no painel.

O script 'porteiro.py' roda no mula-sem-cabeça e fica mandando texto para o painel de LEDs e lendo informação do botão de presença.

Botão
-----

O botão deve ser pressionado pela primeira pessoa que entra no Tarrafa e pela última pessoa que sai do Tarrafa. Ao ligar, o botão manda uma mensagem para o mula-sem-cabeça para que ele atualize o status online do Tarrafa e acende a tomada controlada. Ao desligar, o botão manda novamente a mensagem para o mula-sem-cabeça e prepara a tomada para ser desligada.

Tomada controlada
-----------------

Na tomada controlada ficam ligadas (por enquanto) a luz externa do Tarrafa e a cafeteira. A tomada desliga um minuto depois de o botão ser desligado, dando tempo para que a luz externa ilumine a porta enquanto ela é fechada. A cafeteira também é desligada automaticamente para ninguém esquecer de desligá-la...

Painel de LEDs
--------------

O painel de LEDs foi reutilizado de alguma sucata. Durante encontros da N.E.R.D., foi feita a engenharia reversa do circuito eletrônico e foi feita uma interface para controlá-lo pelo Arduino. O painel tem 10 matrizes de 7x5 LEDs verdes/vermelhos. Dá pra passar qualquer coisa nos 10 caracteres do painel, mas no momento o que passa é a data, a hora e alguma mensagem de auto-ajuda qualquer...

Mood Lamp
---------

A 'mood lamp' fica ligada enquanto tem gente no Tarrafa. As cores da lâmpada ficam variando aleatoriamente num LED RGB.

