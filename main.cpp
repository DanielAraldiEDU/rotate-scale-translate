#include <GLUT/glut.h>
#include <iostream>
#include <cmath>

// Variáveis de transformação
struct Transform
{
  float rotateX = 0.0f;
  float rotateY = 0.0f;
  float rotateZ = 0.0f;
  float scaleX = 1.0f;
  float scaleY = 1.0f;
  float scaleZ = 1.0f;
  float translateX = 0.0f;
  float translateY = 0.0f;
  float translateZ = 0.0f;
} transform;

// Configurações da janela
int windowWidth = 800;
int windowHeight = 600;

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

void initOpenGL()
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);

  // Configuração da luz
  GLfloat lightPos[] = {2.0f, 2.0f, 2.0f, 1.0f};
  GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
  GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
  GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

  glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
}

void drawCube()
{
  glBegin(GL_QUADS);

  // Face frontal (vermelha)
  glColor3f(1.0f, 0.0f, 0.0f);
  glNormal3f(0.0f, 0.0f, 1.0f);
  glVertex3f(-0.5f, -0.5f, 0.5f);
  glVertex3f(0.5f, -0.5f, 0.5f);
  glVertex3f(0.5f, 0.5f, 0.5f);
  glVertex3f(-0.5f, 0.5f, 0.5f);

  // Face traseira (verde)
  glColor3f(0.0f, 1.0f, 0.0f);
  glNormal3f(0.0f, 0.0f, -1.0f);
  glVertex3f(-0.5f, -0.5f, -0.5f);
  glVertex3f(-0.5f, 0.5f, -0.5f);
  glVertex3f(0.5f, 0.5f, -0.5f);
  glVertex3f(0.5f, -0.5f, -0.5f);

  // Face superior (azul)
  glColor3f(0.0f, 0.0f, 1.0f);
  glNormal3f(0.0f, 1.0f, 0.0f);
  glVertex3f(-0.5f, 0.5f, -0.5f);
  glVertex3f(-0.5f, 0.5f, 0.5f);
  glVertex3f(0.5f, 0.5f, 0.5f);
  glVertex3f(0.5f, 0.5f, -0.5f);

  // Face inferior (amarela)
  glColor3f(1.0f, 1.0f, 0.0f);
  glNormal3f(0.0f, -1.0f, 0.0f);
  glVertex3f(-0.5f, -0.5f, -0.5f);
  glVertex3f(0.5f, -0.5f, -0.5f);
  glVertex3f(0.5f, -0.5f, 0.5f);
  glVertex3f(-0.5f, -0.5f, 0.5f);

  // Face direita (magenta)
  glColor3f(1.0f, 0.0f, 1.0f);
  glNormal3f(1.0f, 0.0f, 0.0f);
  glVertex3f(0.5f, -0.5f, -0.5f);
  glVertex3f(0.5f, 0.5f, -0.5f);
  glVertex3f(0.5f, 0.5f, 0.5f);
  glVertex3f(0.5f, -0.5f, 0.5f);

  // Face esquerda (ciano)
  glColor3f(0.0f, 1.0f, 1.0f);
  glNormal3f(-1.0f, 0.0f, 0.0f);
  glVertex3f(-0.5f, -0.5f, -0.5f);
  glVertex3f(-0.5f, -0.5f, 0.5f);
  glVertex3f(-0.5f, 0.5f, 0.5f);
  glVertex3f(-0.5f, 0.5f, -0.5f);

  glEnd();
}

void drawAxes()
{
  glDisable(GL_LIGHTING);
  glLineWidth(3.0f);

  glBegin(GL_LINES);
  // Eixo X (vermelho)
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(1.0f, 0.0f, 0.0f);

  // Eixo Y (verde)
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 1.0f, 0.0f);

  // Eixo Z (azul)
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 1.0f);
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
  gluLookAt(3.0, 3.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

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
  drawCube();

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
    transform.scaleX = transform.scaleY = transform.scaleZ = 1.0f;
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
    if (transform.scaleX > 5.0f)
      transform.scaleX = 5.0f;
    break;
  case SCALE_Y:
    transform.scaleY += deltaX * 0.01f;
    if (transform.scaleY < 0.1f)
      transform.scaleY = 0.1f;
    if (transform.scaleY > 5.0f)
      transform.scaleY = 5.0f;
    break;
  case SCALE_Z:
    transform.scaleZ += deltaX * 0.01f;
    if (transform.scaleZ < 0.1f)
      transform.scaleZ = 0.1f;
    if (transform.scaleZ > 5.0f)
      transform.scaleZ = 5.0f;
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
    if (transform.scaleX > 5.0f)
    {
      transform.scaleX = transform.scaleY = transform.scaleZ = 5.0f;
    }
  }
  break;
  case TRANSLATION_X:
    transform.translateX += deltaX * 0.01f;
    break;
  case TRANSLATION_Y:
    transform.translateY += deltaX * 0.01f;
    break;
  case TRANSLATION_Z:
    transform.translateZ += deltaX * 0.01f;
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
