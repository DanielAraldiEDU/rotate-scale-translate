#include <GLUT/glut.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

// Variáveis de transformação
struct Transform
{
  float rotateX = 0.0f;
  float rotateY = 0.0f;
  float rotateZ = 0.0f;
  float scaleX = 0.5f;
  float scaleY = 0.5f;
  float scaleZ = 0.5f;
  float translateX = 0.0f;
  float translateY = 0.0f;
  float translateZ = 0.0f;
} transform;

// Configurações da janela
int windowWidth = 1000;
int windowHeight = 800;

// Controle de interação
enum TransformMode
{
  ROTATION_X,
  ROTATION_Y,
  ROTATION_Z,
  SCALE_X,
  SCALE_Y,
  SCALE_Z,
  SCALE_UNIFORM,
  TRANSLATION_X,
  TRANSLATION_Y,
  TRANSLATION_Z
} currentMode;

bool mousePressed = false;
int lastMouseX;

struct ObjModel
{
  std::vector<float> vertices;  // x, y, z
  std::vector<float> texcoords; // u, v
  std::vector<float> normals;   // nx, ny, nz

  // Cada face é composta por índices para v, vt, vn
  struct Face
  {
    unsigned int v[3];  // índices dos vértices
    unsigned int vt[3]; // índices das coordenadas de textura
    unsigned int vn[3]; // índices das normais
  };
  std::vector<Face> faces;
};

ObjModel loadObj(const std::string &filename)
{
  ObjModel model;

  std::ifstream file(filename);
  if (!file.is_open())
  {
    std::cerr << "Erro ao abrir o arquivo!" << std::endl;
  }

  std::string line;

  // Para calcular bounding box
  float minX = std::numeric_limits<float>::max();
  float minY = std::numeric_limits<float>::max();
  float minZ = std::numeric_limits<float>::max();
  float maxX = std::numeric_limits<float>::lowest();
  float maxY = std::numeric_limits<float>::lowest();
  float maxZ = std::numeric_limits<float>::lowest();

  std::vector<float> tempVertices;

  while (std::getline(file, line))
  {
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;

    if (prefix == "v")
    {
      float x, y, z;
      iss >> x >> y >> z;
      tempVertices.push_back(x);
      tempVertices.push_back(y);
      tempVertices.push_back(z);
      // Atualiza bounding box
      if (x < minX) minX = x;
      if (y < minY) minY = y;
      if (z < minZ) minZ = z;
      if (x > maxX) maxX = x;
      if (y > maxY) maxY = y;
      if (z > maxZ) maxZ = z;
    }
    else if (prefix == "vt")
    {
      float u, v;
      iss >> u >> v;
      model.texcoords.push_back(u);
      model.texcoords.push_back(v);
    }
    else if (prefix == "vn")
    {
      float nx, ny, nz;
      iss >> nx >> ny >> nz;
      model.normals.push_back(nx);
      model.normals.push_back(ny);
      model.normals.push_back(nz);
    }
    else if (prefix == "f")
    {
      std::vector<unsigned int> vIdx, vtIdx, vnIdx;
      std::string vert;
      while (iss >> vert)
      {
        std::istringstream viss(vert);
        std::string vStr, vtStr, vnStr;
        std::getline(viss, vStr, '/');
        std::getline(viss, vtStr, '/');
        std::getline(viss, vnStr, '/');
        vIdx.push_back(vStr.empty() ? 0 : std::stoi(vStr) - 1);
        vtIdx.push_back(vtStr.empty() ? 0 : std::stoi(vtStr) - 1);
        vnIdx.push_back(vnStr.empty() ? 0 : std::stoi(vnStr) - 1);
      }
      // Triangulação: fan
      for (size_t i = 1; i + 1 < vIdx.size(); ++i)
      {
        ObjModel::Face face;
        face.v[0] = vIdx[0];
        face.vt[0] = vtIdx[0];
        face.vn[0] = vnIdx[0];
        face.v[1] = vIdx[i];
        face.vt[1] = vtIdx[i];
        face.vn[1] = vnIdx[i];
        face.v[2] = vIdx[i + 1];
        face.vt[2] = vtIdx[i + 1];
        face.vn[2] = vnIdx[i + 1];
        model.faces.push_back(face);
      }
    }
  }


  // Normaliza os vértices para que o maior eixo seja 1 e o centro fique na origem
  float sizeX = maxX - minX;
  float sizeY = maxY - minY;
  float sizeZ = maxZ - minZ;
  float maxSize = std::max(std::max(sizeX, sizeY), sizeZ);
  float centerX = (maxX + minX) / 2.0f;
  float centerY = (maxY + minY) / 2.0f;
  float centerZ = (maxZ + minZ) / 2.0f;

  float desiredSize = 75.0f; // Novo tamanho padrão do maior eixo
  for (size_t i = 0; i < tempVertices.size(); i += 3)
  {
    float x = tempVertices[i];
    float y = tempVertices[i + 1];
    float z = tempVertices[i + 2];
    // Centraliza e escala para que o maior eixo seja desiredSize
    x = (x - centerX) / (maxSize / 2.0f) * (desiredSize / 2.0f);
    y = (y - centerY) / (maxSize / 2.0f) * (desiredSize / 2.0f);
    z = (z - centerZ) / (maxSize / 2.0f) * (desiredSize / 2.0f);
    model.vertices.push_back(x);
    model.vertices.push_back(y);
    model.vertices.push_back(z);
  }

  return model;
}

void drawModel(const ObjModel &model)
{
  glBegin(GL_TRIANGLES);
  for (const auto &face : model.faces)
  {
    for (int i = 0; i < 3; ++i)
    {
      // Normais
      if (!model.normals.empty())
      {
        unsigned int ni = face.vn[i] * 3;
        glNormal3f(
            model.normals[ni],
            model.normals[ni + 1],
            model.normals[ni + 2]);
      }

      // Coordenadas de textura
      if (!model.texcoords.empty())
      {
        unsigned int ti = face.vt[i] * 2;
        glTexCoord2f(
            model.texcoords[ti],
            model.texcoords[ti + 1]);
      }

      // Vértices
      unsigned int vi = face.v[i] * 3;
      glVertex3f(
          model.vertices[vi],
          model.vertices[vi + 1],
          model.vertices[vi + 2]);
    }
  }
  glEnd();
}

void initOpenGL()
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  // Configuração da luz
  GLfloat lightPos[] = {10.0f, 10.0f, 10.0f, 1.0f};
  // Luz ambiente mais forte
  GLfloat lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
  // Luz difusa mais forte
  GLfloat lightDiffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};
  GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}

void drawAxes()
{
  glDisable(GL_LIGHTING);
  glLineWidth(5.0f);

  glBegin(GL_LINES);
  // Eixo X (vermelho)
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(25.0f, 0.0f, 0.0f);

  // Eixo Y (verde)
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 25.0f, 0.0f);

  // Eixo Z (azul)
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 25.0f);
  glEnd();

  glEnable(GL_LIGHTING);
}

void displayText()
{
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, windowWidth, 0, windowHeight);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glColor3f(1.0f, 1.0f, 1.0f);

  // Instruções
  const char *instructions[] = {
      "CONTROLES:",
      "1-3: Rotacao X, Y, Z",
      "4-6: Escala X, Y, Z",
      "7: Escala Uniforme",
      "8-0: Translacao X, Y, Z",
      "Mouse: Arraste para ajustar",
      "R: Reset transformacoes",
      "",
      "Modo atual:"};

  const char *modes[] = {
      "Rotacao X", "Rotacao Y", "Rotacao Z",
      "Escala X", "Escala Y", "Escala Z", "Escala Uniforme",
      "Translacao X", "Translacao Y", "Translacao Z"};

  int y = windowHeight - 20;
  for (int i = 0; i < 9; i++)
  {
    glRasterPos2f(10, y);
    for (const char *c = instructions[i]; *c != '\0'; c++)
    {
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    y -= 20;
  }

  // Modo atual
  glColor3f(0.0f, 1.0f, 0.0f);
  glRasterPos2f(10, y);
  for (const char *c = modes[currentMode]; *c != '\0'; c++)
  {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
  }

  // Valores atuais
  char buffer[256];
  y -= 40;
  glColor3f(1.0f, 1.0f, 0.0f);

  sprintf(buffer, "Rotacao: X=%.1f Y=%.1f Z=%.1f",
          transform.rotateX, transform.rotateY, transform.rotateZ);
  glRasterPos2f(10, y);
  for (const char *c = buffer; *c != '\0'; c++)
  {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
  }

  y -= 20;
  sprintf(buffer, "Escala: X=%.2f Y=%.2f Z=%.2f",
          transform.scaleX, transform.scaleY, transform.scaleZ);
  glRasterPos2f(10, y);
  for (const char *c = buffer; *c != '\0'; c++)
  {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
  }

  y -= 20;
  sprintf(buffer, "Translacao: X=%.2f Y=%.2f Z=%.2f",
          transform.translateX, transform.translateY, transform.translateZ);
  glRasterPos2f(10, y);
  for (const char *c = buffer; *c != '\0'; c++)
  {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
  }

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_LIGHTING);
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Configurar câmera
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (double)windowWidth / windowHeight, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(30.0, 30.0, 50.0, 0.0, 0.0, 0.0, 0.0, 100.0, 0.0);

  // Desenhar eixos
  drawAxes();

  // Aplicar transformações (ordem importante!)
  glPushMatrix();

  // 1. Translação (glTranslate)
  glTranslatef(transform.translateX, transform.translateY, transform.translateZ);

  // 2. Rotação (glRotate)
  glRotatef(transform.rotateX, 1.0f, 0.0f, 0.0f);
  glRotatef(transform.rotateY, 0.0f, 1.0f, 0.0f);
  glRotatef(transform.rotateZ, 0.0f, 0.0f, 1.0f);

  // 3. Escala (glScale)
  glScalef(transform.scaleX, transform.scaleY, transform.scaleZ);

  // Desenhar objeto
  ObjModel model = loadObj("./assets/elepham.obj");
  drawModel(model);

  glPopMatrix();

  // Desenhar interface
  displayText();

  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
  case '1':
    currentMode = ROTATION_X;
    break;
  case '2':
    currentMode = ROTATION_Y;
    break;
  case '3':
    currentMode = ROTATION_Z;
    break;
  case '4':
    currentMode = SCALE_X;
    break;
  case '5':
    currentMode = SCALE_Y;
    break;
  case '6':
    currentMode = SCALE_Z;
    break;
  case '7':
    currentMode = SCALE_UNIFORM;
    break;
  case '8':
    currentMode = TRANSLATION_X;
    break;
  case '9':
    currentMode = TRANSLATION_Y;
    break;
  case '0':
    currentMode = TRANSLATION_Z;
    break;
  case 'r':
  case 'R':
    // Reset todas as transformações
    transform.rotateX = transform.rotateY = transform.rotateZ = 0.0f;
    transform.scaleX = transform.scaleY = transform.scaleZ = 0.5f;
    transform.translateX = transform.translateY = transform.translateZ = 0.0f;
    break;
  case 27: // ESC
    exit(0);
    break;
  }
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON)
  {
    if (state == GLUT_DOWN)
    {
      mousePressed = true;
      lastMouseX = x;
    }
    else
    {
      mousePressed = false;
    }
  }
}

void mouseMotion(int x, int y)
{
  if (!mousePressed)
    return;

  float deltaX = (x - lastMouseX) * 0.5f;

  switch (currentMode)
  {
  case ROTATION_X:
    transform.rotateX += deltaX;
    if (transform.rotateX > 360.0f)
      transform.rotateX -= 360.0f;
    if (transform.rotateX < 0.0f)
      transform.rotateX += 360.0f;
    break;
  case ROTATION_Y:
    transform.rotateY += deltaX;
    if (transform.rotateY > 360.0f)
      transform.rotateY -= 360.0f;
    if (transform.rotateY < 0.0f)
      transform.rotateY += 360.0f;
    break;
  case ROTATION_Z:
    transform.rotateZ += deltaX;
    if (transform.rotateZ > 360.0f)
      transform.rotateZ -= 360.0f;
    if (transform.rotateZ < 0.0f)
      transform.rotateZ += 360.0f;
    break;
  case SCALE_X:
    transform.scaleX += deltaX * 0.01f;
    if (transform.scaleX < 0.1f)
      transform.scaleX = 0.1f;
    if (transform.scaleX > 100.0f)
      transform.scaleX = 100.0f;
    break;
  case SCALE_Y:
    transform.scaleY += deltaX * 0.01f;
    if (transform.scaleY < 0.1f)
      transform.scaleY = 0.1f;
    if (transform.scaleY > 100.0f)
      transform.scaleY = 100.0f;
    break;
  case SCALE_Z:
    transform.scaleZ += deltaX * 0.01f;
    if (transform.scaleZ < 0.1f)
      transform.scaleZ = 0.1f;
    if (transform.scaleZ > 100.0f)
      transform.scaleZ = 100.0f;
    break;
  case SCALE_UNIFORM:
  {
    float scale = deltaX * 0.01f;
    transform.scaleX += scale;
    transform.scaleY += scale;
    transform.scaleZ += scale;
    if (transform.scaleX < 0.1f)
    {
      transform.scaleX = transform.scaleY = transform.scaleZ = 0.1f;
    }
    if (transform.scaleX > 100.0f)
    {
      transform.scaleX = transform.scaleY = transform.scaleZ = 100.0f;
    }
  }
  break;
  case TRANSLATION_X:
    transform.translateX += deltaX * 0.1f;
    break;
  case TRANSLATION_Y:
    transform.translateY += deltaX * 0.1f;
    break;
  case TRANSLATION_Z:
    transform.translateZ += deltaX * 0.1f;
    break;
  }

  lastMouseX = x;
  glutPostRedisplay();
}

void reshape(int w, int h)
{
  windowWidth = w;
  windowHeight = h;
  glViewport(0, 0, w, h);
  glutPostRedisplay();
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(windowWidth, windowHeight);
  glutCreateWindow("Interface OpenGL - Transformações 3D");

  initOpenGL();

  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  glutReshapeFunc(reshape);

  std::cout << "=== INTERFACE OPENGL - TRANSFORMAÇÕES 3D ===" << std::endl;
  std::cout << "Controles:" << std::endl;
  std::cout << "1-3: Rotacao X, Y, Z" << std::endl;
  std::cout << "4-6: Escala X, Y, Z" << std::endl;
  std::cout << "7: Escala Uniforme" << std::endl;
  std::cout << "8-0: Translação X, Y, Z" << std::endl;
  std::cout << "R: Reset transformações" << std::endl;
  std::cout << "Mouse: Arraste para ajustar valores" << std::endl;
  std::cout << "ESC: Sair" << std::endl;

  glutMainLoop();
  return 0;
}
