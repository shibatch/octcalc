int main_(int argc, char **argv);

#if defined(_WIN32) && !defined(DEBUG) && !defined(TEST)
#include <vector>
#include <windows.h>
#include <stringapiset.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  int argc = 0;
  LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &argc);

  std::vector<size_t> a(argc);
  std::vector<char> v;

  for (int i = 0;i < argc;i++) {
    a[i] = v.size();
    v.resize(a[i] + WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0, NULL, NULL) + 1);
    WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, v.data() + a[i], int(v.size() - a[i]), NULL, NULL);
  }

  std::vector<char *> argv(argc);
  for(int i=0;i<argc;i++) argv[i] = v.data() + a[i];
  
  return main_(argc, argv.data());
}
#else
#include <cstdio>
#include <QtGlobal>
#include <QByteArray>
#include <QString>

FILE *debugOut = nullptr;

static void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  if (!debugOut) return;
  QByteArray localMsg = msg.toLocal8Bit();
  switch (type) {
  case QtDebugMsg:
    fprintf(debugOut, "Debug: %s\n", localMsg.constData());
    break;
  case QtInfoMsg:
    fprintf(debugOut, "Info: %s\n", localMsg.constData());
    break;
  case QtWarningMsg:
    fprintf(debugOut, "Warning: %s\n", localMsg.constData());
    break;
  case QtCriticalMsg:
    fprintf(debugOut, "Critical: %s\n", localMsg.constData());
    break;
  case QtFatalMsg:
    fprintf(debugOut, "Fatal: %s\n", localMsg.constData());
    abort();
  }
  fflush(debugOut);
}

int main(int argc, char **argv) {
#ifdef DEBUG
  debugOut = fopen("debug.out", "w");
  qInstallMessageHandler(myMessageOutput);
#endif
  int ret = main_(argc, argv);
#ifdef DEBUG
  if (debugOut) fclose(debugOut);
#endif
  return ret;
}
#endif
