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
