#include <GL/glut.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Vec2 {
    float u = 0.0f;
    float v = 0.0f;
};

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct ObjIndex {
    int v = -1;   // indice do vertice
    int vt = -1;  // indice da coordenada de textura
    int vn = -1;  // indice da normal
};

struct Face {
    vector<ObjIndex> pontos;
};

struct Model {
    vector<Vec3> vertices;
    vector<Vec2> texcoords;
    vector<Vec3> normals;
    vector<Face> faces;

    Vec3 center;
    float fitScale = 1.0f;
};

struct BMPImage {
    int width = 0;
    int height = 0;
    vector<unsigned char> data;
};

Model modelo;
GLuint texturaID = 0;
bool texturaLigada = true;
bool texturaCarregada = false;

float rotX = 0.0f;
float rotY = 0.0f;
float rotZ = 0.0f;
float escala = 1.0f;
float transX = 0.0f;
float transY = 0.0f;
float transZ = -5.0f;

bool luzLigada[3] = { true, true, true };
GLfloat posLuzes[3][4] = {
    {  4.0f,  4.0f,  4.0f, 1.0f },
    { -4.0f,  3.0f,  2.0f, 1.0f },
    {  0.0f, -4.0f,  5.0f, 1.0f }
};

int mouseBotao = -1;
int mouseXAnt = 0;
int mouseYAnt = 0;

Vec3 sub(Vec3 a, Vec3 b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

Vec3 cross(Vec3 a, Vec3 b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vec3 normalize(Vec3 v) {
    float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len == 0.0f) return { 0.0f, 0.0f, 1.0f };
    return { v.x / len, v.y / len, v.z / len };
}

int corrigirIndiceOBJ(int indice, int tamanhoVetor) {
    if (indice > 0) return indice - 1;          // OBJ normalmente usa indice comecando em 1
    if (indice < 0) return tamanhoVetor + indice; // indice negativo no OBJ conta a partir do final
    return -1;
}

ObjIndex parseFaceToken(const string& token) {
    ObjIndex idx;

    size_t barra1 = token.find('/');
    if (barra1 == string::npos) {
        idx.v = corrigirIndiceOBJ(stoi(token), (int)modelo.vertices.size());
        return idx;
    }

    string parteV = token.substr(0, barra1);
    if (!parteV.empty()) {
        idx.v = corrigirIndiceOBJ(stoi(parteV), (int)modelo.vertices.size());
    }

    size_t barra2 = token.find('/', barra1 + 1);
    if (barra2 == string::npos) {
        string parteVT = token.substr(barra1 + 1);
        if (!parteVT.empty()) {
            idx.vt = corrigirIndiceOBJ(stoi(parteVT), (int)modelo.texcoords.size());
        }
        return idx;
    }

    string parteVT = token.substr(barra1 + 1, barra2 - barra1 - 1);
    string parteVN = token.substr(barra2 + 1);

    if (!parteVT.empty()) {
        idx.vt = corrigirIndiceOBJ(stoi(parteVT), (int)modelo.texcoords.size());
    }
    if (!parteVN.empty()) {
        idx.vn = corrigirIndiceOBJ(stoi(parteVN), (int)modelo.normals.size());
    }

    return idx;
}

void calcularCentroEEscala() {
    if (modelo.vertices.empty()) return;

    Vec3 minV = modelo.vertices[0];
    Vec3 maxV = modelo.vertices[0];

    for (const Vec3& v : modelo.vertices) {
        minV.x = min(minV.x, v.x);
        minV.y = min(minV.y, v.y);
        minV.z = min(minV.z, v.z);

        maxV.x = max(maxV.x, v.x);
        maxV.y = max(maxV.y, v.y);
        maxV.z = max(maxV.z, v.z);
    }

    modelo.center = {
        (minV.x + maxV.x) / 2.0f,
        (minV.y + maxV.y) / 2.0f,
        (minV.z + maxV.z) / 2.0f
    };

    float largura = maxV.x - minV.x;
    float altura = maxV.y - minV.y;
    float profundidade = maxV.z - minV.z;
    float maiorDimensao = max(largura, max(altura, profundidade));

    if (maiorDimensao > 0.0f) {
        modelo.fitScale = 2.5f / maiorDimensao;
    }
}

bool carregarOBJ(const string& caminho) {
    ifstream arquivo(caminho);
    if (!arquivo.is_open()) {
        cout << "arquivo nao encontrado: " << caminho << endl;
        return false;
    }

    string linha;
    while (getline(arquivo, linha)) {
        if (linha.empty() || linha[0] == '#') continue;

        istringstream iss(linha);
        string tipo;
        iss >> tipo;

        if (tipo == "v") {
            Vec3 v;
            iss >> v.x >> v.y >> v.z;
            modelo.vertices.push_back(v);
        }
        else if (tipo == "vt") {
            Vec2 t;
            iss >> t.u >> t.v;
            modelo.texcoords.push_back(t);
        }
        else if (tipo == "vn") {
            Vec3 n;
            iss >> n.x >> n.y >> n.z;
            modelo.normals.push_back(normalize(n));
        }
        else if (tipo == "f") {
            Face face;
            string token;
            while (iss >> token) {
                face.pontos.push_back(parseFaceToken(token));
            }
            if (face.pontos.size() >= 3) {
                modelo.faces.push_back(face);
            }
        }
    }

    arquivo.close();
    calcularCentroEEscala();

    cout << "OBJ carregado: " << caminho << endl;
    cout << "Vertices: " << modelo.vertices.size() << endl;
    cout << "Texturas vt: " << modelo.texcoords.size() << endl;
    cout << "Normais vn: " << modelo.normals.size() << endl;
    cout << "Faces: " << modelo.faces.size() << endl;

    return true;
}

bool carregarBMP24(const string& caminho, BMPImage& img) {
    ifstream arq(caminho, ios::binary);
    if (!arq.is_open()) return false;

    unsigned char header[54];
    arq.read((char*)header, 54);

    if (header[0] != 'B' || header[1] != 'M') return false;

    int offset = *(int*)&header[10];
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    short bitsPorPixel = *(short*)&header[28];

    if (bitsPorPixel != 24) {
        cout << "Erro: a textura precisa ser BMP de 24 bits." << endl;
        return false;
    }

    int absHeight = abs(height);
    int rowSize = ((width * 3 + 3) / 4) * 4;
    vector<unsigned char> raw(rowSize * absHeight);

    arq.seekg(offset);
    arq.read((char*)raw.data(), raw.size());
    arq.close();

    img.width = width;
    img.height = absHeight;
    img.data.resize(width * absHeight * 3);

    for (int y = 0; y < absHeight; y++) {
        int srcY = (height > 0) ? (absHeight - 1 - y) : y;
        for (int x = 0; x < width; x++) {
            int src = srcY * rowSize + x * 3;
            int dst = (y * width + x) * 3;

            img.data[dst + 0] = raw[src + 2]; // R
            img.data[dst + 1] = raw[src + 1]; // G
            img.data[dst + 2] = raw[src + 0]; // B
        }
    }

    return true;
}

void criarTexturaXadrez() {
    const int W = 64;
    const int H = 64;
    vector<unsigned char> data(W * H * 3);

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int c = (((x / 8) + (y / 8)) % 2) ? 255 : 40;
            int i = (y * W + x) * 3;
            data[i + 0] = (unsigned char)c;
            data[i + 1] = (unsigned char)c;
            data[i + 2] = (unsigned char)c;
        }
    }

    glBindTexture(GL_TEXTURE_2D, texturaID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
}

void configurarTextura(const string& caminhoTextura) {
    glGenTextures(1, &texturaID);
    glBindTexture(GL_TEXTURE_2D, texturaID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    BMPImage img;
    if (carregarBMP24(caminhoTextura, img)) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, img.data.data());
        texturaCarregada = true;
        cout << "Textura BMP carregada: " << caminhoTextura << endl;
    }
    else {
        criarTexturaXadrez();
        texturaCarregada = true;
        cout << "Textura BMP nao encontrada. Usando textura xadrez procedural." << endl;
    }
}

void configurarUmaLuz(GLenum luz, int i) {
    GLfloat ambiente[4]  = { 0.15f, 0.15f, 0.15f, 1.0f };
    GLfloat difusa[4]    = { 0.75f, 0.75f, 0.75f, 1.0f };
    GLfloat especular[4] = { 1.00f, 1.00f, 1.00f, 1.0f };

    if (i == 1) {
        difusa[0] = 0.55f; difusa[1] = 0.65f; difusa[2] = 1.00f;
    }
    if (i == 2) {
        difusa[0] = 1.00f; difusa[1] = 0.70f; difusa[2] = 0.55f;
    }

    glLightfv(luz, GL_AMBIENT, ambiente);
    glLightfv(luz, GL_DIFFUSE, difusa);
    glLightfv(luz, GL_SPECULAR, especular);
    glLightfv(luz, GL_POSITION, posLuzes[i]);
}

void atualizarLuzes() {
    GLenum luzes[3] = { GL_LIGHT0, GL_LIGHT1, GL_LIGHT2 };

    for (int i = 0; i < 3; i++) {
        if (luzLigada[i]) {
            glEnable(luzes[i]);
            configurarUmaLuz(luzes[i], i);
        }
        else {
            glDisable(luzes[i]);
        }
    }
}

void desenharMarcadoresDasLuzes() {
    glDisable(GL_LIGHTING);

    for (int i = 0; i < 3; i++) {
        if (!luzLigada[i]) continue;
        glPushMatrix();
        glTranslatef(posLuzes[i][0], posLuzes[i][1], posLuzes[i][2]);
        glColor3f(1.0f, 1.0f, 1.0f);
        glutSolidSphere(0.08, 12, 12);
        glPopMatrix();
    }

    glEnable(GL_LIGHTING);
}

void configurarMaterial() {
    GLfloat ambiente[]  = { 0.25f, 0.25f, 0.25f, 1.0f };
    GLfloat difusa[]    = { 0.80f, 0.80f, 0.80f, 1.0f };
    GLfloat especular[] = { 1.00f, 1.00f, 1.00f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, difusa);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64.0f);
}

bool indiceValidoVertice(const ObjIndex& idx) {
    return idx.v >= 0 && idx.v < (int)modelo.vertices.size();
}

void enviarVertice(const ObjIndex& idx, Vec3 normalFace) {
    if (idx.vn >= 0 && idx.vn < (int)modelo.normals.size()) {
        Vec3 n = modelo.normals[idx.vn];
        glNormal3f(n.x, n.y, n.z);
    }
    else {
        glNormal3f(normalFace.x, normalFace.y, normalFace.z);
    }

    if (idx.vt >= 0 && idx.vt < (int)modelo.texcoords.size()) {
        Vec2 t = modelo.texcoords[idx.vt];
        glTexCoord2f(t.u, t.v);
    }

    Vec3 v = modelo.vertices[idx.v];
    glVertex3f(v.x, v.y, v.z);
}

void desenharModelo() {
    configurarMaterial();

    bool podeUsarTextura = texturaLigada && texturaCarregada && !modelo.texcoords.empty();

    if (podeUsarTextura) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaID);
    }
    else {
        glDisable(GL_TEXTURE_2D);
    }

    glBegin(GL_TRIANGLES);

    for (const Face& face : modelo.faces) {
        if (face.pontos.size() < 3) continue;

        // Triangulacao em leque: transforma faces com 4 ou mais vertices em varios triangulos.
        for (size_t i = 1; i + 1 < face.pontos.size(); i++) {
            ObjIndex a = face.pontos[0];
            ObjIndex b = face.pontos[i];
            ObjIndex c = face.pontos[i + 1];

            if (!indiceValidoVertice(a) || !indiceValidoVertice(b) || !indiceValidoVertice(c)) {
                continue;
            }

            Vec3 va = modelo.vertices[a.v];
            Vec3 vb = modelo.vertices[b.v];
            Vec3 vc = modelo.vertices[c.v];
            Vec3 normalFace = normalize(cross(sub(vb, va), sub(vc, va)));

            enviarVertice(a, normalFace);
            enviarVertice(b, normalFace);
            enviarVertice(c, normalFace);
        }
    }

    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0.0, 0.0, 0.0,
              0.0, 0.0, -1.0,
              0.0, 1.0, 0.0);

    atualizarLuzes();
    desenharMarcadoresDasLuzes();

    glPushMatrix();
    glTranslatef(transX, transY, transZ);
    glRotatef(rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);
    glRotatef(rotZ, 0.0f, 0.0f, 1.0f);
    glScalef(escala * modelo.fitScale, escala * modelo.fitScale, escala * modelo.fitScale);
    glTranslatef(-modelo.center.x, -modelo.center.y, -modelo.center.z);

    desenharModelo();

    glPopMatrix();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, aspect, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

void resetarTransformacoes() {
    rotX = rotY = rotZ = 0.0f;
    escala = 1.0f;
    transX = transY = 0.0f;
    transZ = -5.0f;
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27:
        exit(0);
        break;
    case 'r':
    case 'R':
        resetarTransformacoes();
        break;
    case '+':
    case '=':
        escala *= 1.1f;
        break;
    case '-':
    case '_':
        escala /= 1.1f;
        break;
    case 'a':
    case 'A':
        transX -= 0.15f;
        break;
    case 'd':
    case 'D':
        transX += 0.15f;
        break;
    case 'w':
    case 'W':
        transY += 0.15f;
        break;
    case 's':
    case 'S':
        transY -= 0.15f;
        break;
    case 'q':
    case 'Q':
        transZ += 0.25f;
        break;
    case 'e':
    case 'E':
        transZ -= 0.25f;
        break;
    case 'z':
    case 'Z':
        rotZ -= 5.0f;
        break;
    case 'x':
    case 'X':
        rotZ += 5.0f;
        break;
    case 't':
    case 'T':
        texturaLigada = !texturaLigada;
        break;
    case '1':
        luzLigada[0] = !luzLigada[0];
        break;
    case '2':
        luzLigada[1] = !luzLigada[1];
        break;
    case '3':
        luzLigada[2] = !luzLigada[2];
        break;
    default:
        break;
    }

    glutPostRedisplay();
}

void specialKeyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        rotY -= 5.0f;
        break;
    case GLUT_KEY_RIGHT:
        rotY += 5.0f;
        break;
    case GLUT_KEY_UP:
        rotX -= 5.0f;
        break;
    case GLUT_KEY_DOWN:
        rotX += 5.0f;
        break;
    case GLUT_KEY_PAGE_UP:
        transZ += 0.25f;
        break;
    case GLUT_KEY_PAGE_DOWN:
        transZ -= 0.25f;
        break;
    default:
        break;
    }

    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == 3 && state == GLUT_DOWN) { // roda do mouse para cima
        escala *= 1.05f;
        glutPostRedisplay();
        return;
    }
    if (button == 4 && state == GLUT_DOWN) { // roda do mouse para baixo
        escala /= 1.05f;
        glutPostRedisplay();
        return;
    }

    if (state == GLUT_DOWN) {
        mouseBotao = button;
        mouseXAnt = x;
        mouseYAnt = y;
    }
    else {
        mouseBotao = -1;
    }
}

void motion(int x, int y) {
    int dx = x - mouseXAnt;
    int dy = y - mouseYAnt;

    if (mouseBotao == GLUT_LEFT_BUTTON) {
        rotY += dx * 0.5f;
        rotX += dy * 0.5f;
    }
    else if (mouseBotao == GLUT_RIGHT_BUTTON) {
        transX += dx * 0.01f;
        transY -= dy * 0.01f;
    }
    else if (mouseBotao == GLUT_MIDDLE_BUTTON) {
        escala += dy * 0.005f;
        if (escala < 0.05f) escala = 0.05f;
    }

    mouseXAnt = x;
    mouseYAnt = y;
    glutPostRedisplay();
}

void initGL(const string& caminhoTextura) {
    glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE); // importante quando usa glScale, para normal nao ficar errada
    glShadeModel(GL_SMOOTH);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    configurarTextura(caminhoTextura);
}

void imprimirControles() {
    cout << "\nControles:\n";
    cout << "Setas: rotacionar em X/Y\n";
    cout << "Z/X: rotacionar em Z\n";
    cout << "WASD: transladar em X/Y\n";
    cout << "Q/E ou PageUp/PageDown: aproximar/afastar\n";
    cout << "+/-: escala\n";
    cout << "Mouse esquerdo: rotacao\n";
    cout << "Mouse direito: translacao\n";
    cout << "Scroll: escala\n";
    cout << "1, 2, 3: ligar/desligar luzes\n";
    cout << "T: ligar/desligar textura\n";
    cout << "R: resetar transformacoes\n";
    cout << "ESC: sair\n\n";
}

int main(int argc, char** argv) {
    string caminhoOBJ = "data/mba1.obj";
    string caminhoTextura = "Textures/launch.bmp";

    if (argc >= 2) caminhoOBJ = argv[1];
    if (argc >= 3) caminhoTextura = argv[2];

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(900, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Trabalho 3D - OBJ com normais, textura e iluminacao");

    if (!carregarOBJ(caminhoOBJ)) {
        return 1;
    }

    initGL(caminhoTextura);
    imprimirControles();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutMainLoop();
    return 0;
}
