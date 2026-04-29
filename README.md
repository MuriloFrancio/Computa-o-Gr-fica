# Trabalho 3D - OpenGL, OBJ, Textura e Iluminação

Projeto desenvolvido em C++ usando OpenGL/GLUT para carregar e renderizar um modelo 3D a partir de um arquivo `.obj`.

O programa carrega vértices, faces, normais e coordenadas de textura, renderizando o objeto com faces triangulares, textura BMP e iluminação.

## Funcionalidades

- Carregamento de modelo `.obj`
- Leitura de:
  - vértices (`v`)
  - coordenadas de textura (`vt`)
  - normais (`vn`)
  - faces (`f`)
- Renderização com triângulos
- Aplicação de textura `.bmp`
- Iluminação com 3 pontos de luz
- Luz ambiente, difusa e especular
- Ativar e desativar luzes pelo teclado
- Controle de rotação, escala e translação por teclado e mouse

Requisitos

Para compilar no Windows, foi utilizado:

MSYS2 UCRT64
g++
freeglut

Execute o programa passando o arquivo .obj e a textura .bmp:

./Trabalho3D_OBJ.exe data/mba1.obj Textures/launch1.bmp

Controles
Tecla / Mouse	Ação
Setas	Rotaciona o objeto em X e Y
Z / X	Rotaciona o objeto em Z
W / A / S / D	Move o objeto na tela
Q / E	Aproxima ou afasta o objeto
+ / -	Aumenta ou diminui a escala
Scroll do mouse	Aumenta ou diminui a escala
Mouse esquerdo	Rotaciona o objeto
Mouse direito	Move o objeto
1	Liga/desliga a luz 1
2	Liga/desliga a luz 2
3	Liga/desliga a luz 3
T	Liga/desliga a textura
R	Reseta as transformações
ESC	Fecha o programa
