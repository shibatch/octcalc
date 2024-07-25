#include <cstdio>
#include <cmath>
#include <memory>
#include <unordered_map>
#include <string>
#include <QtWidgets>
#include <QApplication>
#include <QWidget>
#include <QLineEdit>
#include <QToolButton>
#include <QColor>
#include <QFontDatabase>

#include "octcore.hpp"

using namespace std;

class Button : public QToolButton {
public:
  explicit Button(const QString &text, QWidget *parent = 0) : QToolButton(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setText(text);
  }

  QSize sizeHint() const override {
    QSize size = QToolButton::sizeHint();
    size.rheight() += 20;
    size.rwidth() = qMax(size.width(), size.height());
    return size;
  }

  void setColor_(const QColor& c) {
    QPalette pal = palette();
    pal.setColor(QPalette::Button, c);
    setAutoFillBackground(true);
    setPalette(pal);
  }
};

class OctCalc : public QWidget {
public:
  OctCalc(QWidget *parent, QApplication *app_);
  ~OctCalc() { shuttingDown = true; }

private:
  const QApplication *app;
  bool eventFilter(QObject *obj, QEvent *event);
  void processButtonPress(const string &s);

  shared_ptr<Button> createButton(const QString &text, const QColor& color = QColor());

  shared_ptr<QGridLayout> mainLayout;
  shared_ptr<QLineEdit> display;
  shared_ptr<QLabel> label;
  shared_ptr<QFrame> vline0, vline1, vline2, hline0;

  const QColor bdef = QColor(220, 220, 220), red = QColor(220, 120, 120), green = QColor(170, 220, 170), blue = QColor(100, 140, 250);

  octcore::OctCore octCore;

  vector<string> history;
  int histPos = -1;

  bool shuttingDown = false;
  unordered_map<string, shared_ptr<Button>> buttons;
  int displayWidth = -1;
  vector<char> displayBuffer;
  bool modeShift = 0, modeAlt = 0, modeHex = 0, modeInt = 0;

  string displayString = "", subdisplayString = "";
  tlfloat_octuple displayNumber = 0;
  bool showingResult = false;

  bool selectAll = false;

#ifdef TEST
public:
  int doTest();
#endif
};

OctCalc::OctCalc(QWidget *parent, QApplication *app_) : QWidget(parent), app(app_) {
  mainLayout = make_shared<QGridLayout>();
  mainLayout->setSizeConstraint(QLayout::SetFixedSize);

  display = make_shared<QLineEdit>("");
  display->setReadOnly(false);
  display->setAlignment(Qt::AlignRight);
  display->setMaxLength(72);
  {
    QPalette pal = palette();
    pal.setColor(QPalette::Highlight, QColor(220, 220, 220));
    pal.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
    display->setPalette(pal);
  }

#if defined(_WIN32)
  QFont font("Monospace");
  font.setStyleHint(QFont::TypeWriter);
#else
  QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
#endif
  font.setPointSize(display->font().pointSize() + 2);
  display->setFont(font);

  mainLayout->addWidget(display.get(), 1, 0, 1, 14);

  string version = to_string(OCTCALC_VERSION_MAJOR) + "." + to_string(OCTCALC_VERSION_MINOR) + "." + to_string(OCTCALC_VERSION_PATCHLEVEL);
  subdisplayString = "Octuple-Precision Calculator  Version " + version + "  Copyright Naoki Shibata";
  label = make_shared<QLabel>(subdisplayString.c_str());
  label->setAlignment(Qt::AlignRight);

  mainLayout->addWidget(label.get(), 0, 0, 1, 14);

  hline0 = make_shared<QFrame>();
  hline0->setFrameShape(QFrame::HLine);
  hline0->setStyleSheet("border: 0px solid black");
  mainLayout->addWidget(hline0.get(), 2, 0, 1, 0);

  vline0 = make_shared<QFrame>();
  vline0->setFrameShape(QFrame::VLine);
  vline0->setStyleSheet("border: 0px solid black");
  mainLayout->addWidget(vline0.get(), 0, 1, 0, 1);

  vline1 = make_shared<QFrame>();
  vline1->setFrameShape(QFrame::VLine);
  vline1->setStyleSheet("border: 0px solid black");
  mainLayout->addWidget(vline1.get(), 0, 4, 0, 1);

  vline2 = make_shared<QFrame>();
  vline2->setFrameShape(QFrame::VLine);
  vline2->setStyleSheet("border: 0px solid black");
  mainLayout->addWidget(vline2.get(), 0, 8, 0, 1);

  typedef struct { const char *label; int x, y, w, h; QColor color; } butelm;
  const QColor dc = QColor(160, 160, 160);
  static butelm butdefs[] = {
    { "PASTE", 0, 3, 1, 1, green }, { "log10" , 2, 3, 1, 1 }, { "asinh", 3, 3, 1, 1 },
    { "HEX"  , 0, 4, 1, 1, green }, { "exp10" , 2, 4, 1, 1 }, { "acosh", 3, 4, 1, 1 },
    { "DOWN" , 0, 5, 1, 1, green }, { "erf"   , 2, 5, 1, 1 }, { "atanh", 3, 5, 1, 1 },
    { "ALT"  , 0, 6, 1, 1, green }, { "M_PI"  , 2, 6, 1, 1 }, { "tgamma" , 3, 6, 1, 1 },
    { "SHIFT", 0, 7, 1, 1, green }, { "x"     , 2, 7, 1, 1 }, { "="    , 3, 7, 1, 1 },

    { "~"  , 5, 3, 1, 1 }, { "<<", 6, 3, 1, 1     }, { ">>", 7, 3, 1, 1     },
    { "&&" , 5, 4, 1, 1 }, { "0x", 6, 4, 1, 1     }, { "p" , 7, 4, 1, 1     },
    { "|"  , 5, 5, 1, 1 }, { "E" , 6, 5, 1, 1, dc }, { "F" , 7, 5, 1, 1, dc },
    { "^"  , 5, 6, 1, 1 }, { "C" , 6, 6, 1, 1, dc }, { "D" , 7, 6, 1, 1, dc },
    { "gcd", 5, 7, 1, 1 }, { "A" , 6, 7, 1, 1, dc }, { "B" , 7, 7, 1, 1, dc },

    { "hypot", 9, 3, 1, 1       }, { "(" , 10, 3, 1, 1     }, { "," , 11, 3, 1, 1     }, { ")" , 12, 3, 1, 1     }, { "/"    , 13, 3, 1, 1 },
    { "fmod" , 9, 4, 1, 1       }, { "7" , 10, 4, 1, 1, dc }, { "8" , 11, 4, 1, 1, dc }, { "9" , 12, 4, 1, 1, dc }, { "*"    , 13, 4, 1, 1 },
    { "trunc", 9, 5, 1, 1       }, { "4" , 10, 5, 1, 1, dc }, { "5" , 11, 5, 1, 1, dc }, { "6" , 12, 5, 1, 1, dc }, { "-"    , 13, 5, 1, 1 },
    { "BS"  , 9, 6, 1, 1, green}, { "1" , 10, 6, 1, 1, dc }, { "2" , 11, 6, 1, 1, dc }, { "3" , 12, 6, 1, 1, dc }, { "+"    , 13, 6, 1, 1 },
    { "CE"  , 9, 7, 1, 1, red  }, { "0" , 10, 7, 1, 1, dc }, { "." , 11, 7, 1, 1, dc }, { "e" , 12, 7, 1, 1     }, { "ENTER", 13, 7, 1, 1, green },
  };

  for(int i=0;i<sizeof(butdefs)/sizeof(butdefs[0]);i++) {
    auto b = createButton(QString(butdefs[i].label), butdefs[i].color);
    buttons[butdefs[i].label] = b;
    mainLayout->addWidget(b.get(), butdefs[i].y, butdefs[i].x, butdefs[i].h, butdefs[i].w);
  }

  setLayout(mainLayout.get());
  setWindowTitle(tr("OctCalc"));
}

shared_ptr<Button> OctCalc::createButton(const QString &text, const QColor& c) {
  shared_ptr<Button> button = make_shared<Button>(text);
  button->setColor_(c.value() != 0 ? c : bdef);
  return button;
}

//

void OctCalc::processButtonPress(const string &s) {
#ifdef DEBUG
  qDebug() << "processButtonPress : " << s.c_str();
#endif

  int cursorPos = display->cursorPosition();
  int selectionStart = display->selectionStart(), selectionEnd = display->selectionEnd();

  //

  if (s == "Enter" || s == "Return" || s == "ENTER" || s == "HEX" || s == "INT") {
    bool error = false;

    if (s == "HEX") modeHex = !modeHex;
    if (s == "INT") modeInt = !modeInt;

    if (!(s == "HEX" || s == "INT") || !showingResult) {
      subdisplayString = displayString;
      history.push_back(displayString);
      histPos = -1;
      pair<string, tlfloat_octuple> p = octCore.execute(displayString);
      if (p.first.substr(0, 6) == "ERROR:") {
	displayString = p.first.substr(6);
	displayNumber = 0;
	error = true;
      } else {
	displayNumber = p.second;
      }
      showingResult = true;
    }

    if (!error) {
      if (modeHex) {
	if (modeInt) {
	  if (displayNumber <= -tlfloat_ldexpo(1, 127) || tlfloat_ldexpo(1, 127) <= displayNumber) {
	    tlfloat_snprintf(displayBuffer.data(), displayBuffer.size()-1, "OVERFLOW");
	  } else {
	    tlfloat_snprintf(displayBuffer.data(), displayBuffer.size()-1, "0x%Qx", (tlfloat_int128_t)displayNumber);
	  }
	} else {
	  if (tlfloat_snprintf(displayBuffer.data(), displayBuffer.size()-1, "%Oa", displayNumber) > displayWidth) {
	    for(int i=displayWidth;i>=0;i--) {
	      if (tlfloat_snprintf(displayBuffer.data(), displayBuffer.size()-1, "%.*Oa", i, displayNumber) <= displayWidth) break;
	    }
	  }
	}
      } else {
	if (modeInt) {
	  if (displayNumber <= -tlfloat_ldexpo(1, 127) || tlfloat_ldexpo(1, 127) <= displayNumber) {
	    tlfloat_snprintf(displayBuffer.data(), displayBuffer.size()-1, "OVERFLOW");
	  } else {
	    tlfloat_snprintf(displayBuffer.data(), displayBuffer.size()-1, "%Qd", (tlfloat_int128_t)displayNumber);
	  }
	} else {
	  for(int i=displayWidth > 70 ? 70 : displayWidth;i>=0;i--) {
	    if (tlfloat_snprintf(displayBuffer.data(), displayBuffer.size()-1, "%.*Og", i, displayNumber) <= displayWidth) break;
	  }
	}
      }
      displayString = displayBuffer.data();
    }
    selectAll = true;
  } else if (s == "" || s == "SHOW") {
    // Key press on display
    showingResult = false;
  } else if (s == "CE") {
    displayString = "";
    displayNumber = 0;
    showingResult = false;
    histPos = -1;
  } else if (s == "AC") {
    displayString = "";
    displayNumber = 0;
    showingResult = false;
    octCore.clear();
    history.clear();
    histPos = -1;
  } else if (s == "SHIFT") {
    modeShift = !modeShift;
  } else if (s == "ALT") {
    modeAlt = !modeAlt;
  } else if (s == "Up" || s == "UP") {
    if (histPos == -1) {
      histPos = int(history.size()) - 1;
    } else if (histPos > 0) {
      histPos--;
    }
    if (histPos >= 0 && histPos < history.size()) displayString = history[histPos];
    showingResult = false;
    selectAll = true;
  } else if (s == "Down" || s == "DOWN") {
    if (histPos != -1 && histPos < history.size() - 1) histPos++;
    if (histPos >= 0 && histPos < history.size()) displayString = history[histPos];
    showingResult = false;
    selectAll = true;
  } else if (s == "COPY") {
    app->clipboard()->setText(displayString.c_str());
  } else if (s == "PASTE") {
    displayString = app->clipboard()->text().toStdString();
    showingResult = false;
    selectAll = true;
  } else if (s == "WEB") {
    QDesktopServices::openUrl(QUrl("https://github.com/shibatch/octcalc", QUrl::TolerantMode));
  } else if (s == "BS") {
    if (selectionStart == -1) {
      if (cursorPos > 0) {
	displayString = displayString.substr(0, cursorPos-1) +
	  displayString.substr(cursorPos, displayString.size() - cursorPos);
	cursorPos --;
      }
    } else {
      displayString = displayString.substr(0, selectionStart) +
	displayString.substr(selectionEnd, displayString.size() - selectionEnd);
      cursorPos = selectionStart;
      selectionStart = -1;
    }
    showingResult = false;
    histPos = -1;
  } else {
    string t = s;
    if (s == "&&") t = "&";
    if (s == "rem") t = "remainder";
    if (t[0] >= 'a' && t[0] <= 'z' && t.size() > 1) t += "(";
    if (selectionStart == -1) {
      displayString = displayString.substr(0, cursorPos) + t +
	displayString.substr(cursorPos, displayString.size() - cursorPos);
      cursorPos += (int)t.size();
    } else {
      displayString = displayString.substr(0, selectionStart) + t +
	displayString.substr(selectionEnd, displayString.size() - selectionEnd);
      cursorPos = selectionStart + (int)t.size();
      selectionStart = -1;
    }
    showingResult = false;
    histPos = -1;
  }

  //

  if (modeShift) {
    buttons["SHIFT"]->setColor_(blue);
    buttons["HEX"]->setText("UP");
    buttons["DOWN"]->setText("DOWN");
    buttons["fmod"]->setText("rem");
    buttons["M_PI"]->setText("M_E");
    buttons["erf"]->setText("erfc");
    buttons["gcd"]->setText("lcm");
    if (modeAlt) {
      buttons["PASTE"]->setText("WEB");
      buttons["hypot"]->setText("hypot");
      buttons["trunc"]->setText("ceil");
      buttons["asinh"]->setText("asinh");
      buttons["acosh"]->setText("acosh");
      buttons["atanh"]->setText("atanh");
      buttons["log10"]->setText("log1p");
      buttons["exp10"]->setText("expm1");
      buttons["tgamma"]->setText("tgamma");
      buttons["CE" ]->setText("AC");
      buttons["x"  ]->setText("w");
      buttons["="  ]->setText("*=");
    } else {
      buttons["PASTE"]->setText("PASTE");
      buttons["hypot"]->setText("cbrt");
      buttons["trunc"]->setText("rint");
      buttons["asinh"]->setText("asin");
      buttons["acosh"]->setText("acos");
      buttons["atanh"]->setText("atan");
      buttons["log10"]->setText("log10");
      buttons["exp10"]->setText("exp10");
      buttons["tgamma"]->setText("fabs");
      buttons["CE" ]->setText("CE");
      buttons["x"  ]->setText("y");
      buttons["="  ]->setText("+=");
    }
  } else {
    buttons["SHIFT"]->setColor_(green);
    buttons["HEX"]->setText("HEX");
    buttons["DOWN"]->setText("INT");
    buttons["fmod"]->setText("fmod");
    buttons["M_PI"]->setText("M_PI");
    buttons["erf"]->setText("erf");
    buttons["gcd"]->setText("gcd");
    if (modeAlt) {
      buttons["PASTE"]->setText("WEB");
      buttons["hypot"]->setText("pow");
      buttons["trunc"]->setText("floor");
      buttons["asinh"]->setText("sinh");
      buttons["acosh"]->setText("cosh");
      buttons["atanh"]->setText("tanh");
      buttons["log10"]->setText("log2");
      buttons["exp10"]->setText("exp2");
      buttons["tgamma"]->setText("tgamma");
      buttons["CE" ]->setText("CE");
      buttons["x"  ]->setText("z");
      buttons["="  ]->setText("-=");
    } else {
      buttons["PASTE"]->setText("COPY");
      buttons["hypot"]->setText("sqrt");
      buttons["trunc"]->setText("trunc");
      buttons["asinh"]->setText("sin");
      buttons["acosh"]->setText("cos");
      buttons["atanh"]->setText("tan");
      buttons["log10"]->setText("log");
      buttons["exp10"]->setText("exp");
      buttons["tgamma"]->setText("atan2");
      buttons["CE" ]->setText("CE");
      buttons["x"  ]->setText("x");
      buttons["="  ]->setText("=");
    }
  }

  if (modeAlt) {
    buttons["ALT"]->setColor_(blue);
  } else {
    buttons["ALT"]->setColor_(green);
  }

  if (modeHex && !modeShift) {
    buttons["HEX"]->setColor_(blue);
  } else {
    buttons["HEX"]->setColor_(green);
  }

  if (modeInt && !modeShift) {
    buttons["DOWN"]->setColor_(blue);
  } else {
    buttons["DOWN"]->setColor_(green);
  }

  display->setText(displayString.c_str());
  display->setCursorPosition(cursorPos);
  if (selectAll) {
    display->setCursorPosition((int)displayString.size());
    display->selectAll();
    selectAll = false;
  } else if (selectionStart != -1) {
    display->setSelection(selectionStart, selectionEnd - selectionStart);
  }
  if (!showingResult && s != "SHOW") subdisplayString = "";
  label->setText(subdisplayString.c_str());
}

bool OctCalc::eventFilter(QObject *obj, QEvent *event) {
  if (shuttingDown) return QObject::eventFilter(obj, event);

  static string validKeys = "0123456789aAbBcCdDeEfF+-*/^=pPxX()%";

  if (obj == display.get()) displayString = display->text().toStdString();
  if (obj == label.get()) subdisplayString = label->text().toStdString();

  if (obj == this && event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
#ifdef DEBUG
    qDebug() << "Keypress : " << keyEvent->key() << " : " << (char)keyEvent->key() << " from " << obj;
#endif
    switch(keyEvent->key()) {
    case Qt::Key_Backspace: processButtonPress("Backspace"); return true;
    case Qt::Key_Enter: processButtonPress("Enter"); return true;
    case Qt::Key_Return: processButtonPress("Return"); return true;
    case Qt::Key_Delete: processButtonPress("Delete"); return true;
    case Qt::Key_Home: processButtonPress("Home"); return true;
    case Qt::Key_Escape: processButtonPress("Escape"); return true;
    case Qt::Key_Up: processButtonPress("Up"); return true;
    case Qt::Key_Down: processButtonPress("Down"); return true;

    default: 
      if ((int(keyEvent->key()) & ~0xff) == 0 &&
	  validKeys.find_first_of((char)keyEvent->key()) != string::npos) {
	processButtonPress(string("") + (char)keyEvent->key());
	return true;
      }
    }
  } else if (obj == display.get() && event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
#ifdef DEBUG
    qDebug() << "Keypress : " << keyEvent->key() << " : " << (char)keyEvent->key() << " from " << obj;
#endif
    switch(keyEvent->key()) {
    case Qt::Key_Enter: processButtonPress("Enter"); return true;
    case Qt::Key_Return: processButtonPress("Return"); return true;
    case Qt::Key_Home: processButtonPress("Home"); return true;
    case Qt::Key_Up: processButtonPress("Up"); return true;
    case Qt::Key_Down: processButtonPress("Down"); return true;
    }
    processButtonPress("");
  } else if (event->type() == QEvent::MouseButtonRelease) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    Button *clickedButton = dynamic_cast<Button *>(obj);
    if (clickedButton) processButtonPress(clickedButton->text().toStdString());
#ifdef DEBUG
    if (clickedButton) qDebug() << "MouseButtonRelease : " << clickedButton->text().toStdString().c_str();
#endif
  } else if (obj == this && event->type() == QEvent::Show) {
    int w = display->size().width();
    w -= display->textMargins().left() + display->textMargins().right();
    w -= display->contentsMargins().left() + display->contentsMargins().right();
    w -= 8; // Hard coded in qt

#if 0
    int cw = display->fontMetrics().boundingRect('0').width() +
      display->fontMetrics().leftBearing('0') + display->fontMetrics().rightBearing('0');
    displayWidth = w / cw;
#else
    string z = "";
    while(display->fontMetrics().boundingRect(z.c_str()).width() < w) z += "0";
    displayWidth = z.size() - 1;
#endif
    displayBuffer.resize(displayWidth+10);

    display->setMaxLength(displayWidth);
    label->setMaximumWidth(w);

    for(auto b : buttons) {
      b.second->setMinimumWidth(b.second->size().width());
      b.second->setMaximumWidth(b.second->size().width());
      b.second->setMinimumHeight(b.second->size().height());
      b.second->setMaximumHeight(b.second->size().height());
    }

    processButtonPress("SHOW");

#ifdef DEBUG
    processButtonPress(string("0xxxxxxxxx1xxxxxxxxx2xxxxxxxxx3xxxxxxxxx4xxxxxxxxx5xxxxxxxxx6xxxxxxxxx7xxxxxxxxx8xxxxxxxxx9"));

    qDebug() << "display->size().width() = " << display->size().width();
    qDebug() << "display->textMargins().left() = " << display->textMargins().left();
    qDebug() << "display->contentsMargins().left() = " << display->contentsMargins().left();
    qDebug() << "w = " << w;
    qDebug() << "display->fontMetrics().boundingRect('0').width() = " << display->fontMetrics().boundingRect('0').width();
    qDebug() << "display->fontMetrics().leftBearing('0') = " << display->fontMetrics().leftBearing('0');
    qDebug() << "display->fontMetrics().rightBearing('0') = " << display->fontMetrics().rightBearing('0');
    qDebug() << "length of display : " << displayWidth;
#endif
  }

  return QObject::eventFilter(obj, event);
}

//

#ifdef TEST
#include <QtTest/QTest>

int OctCalc::doTest() {
  try {
    QTest::keyClicks(display.get(), "M_PI");
    qDebug() << "0: " << display->text();
    if (display->text() != QString("M_PI")) throw(runtime_error("0: key click \"M_PI\""));

    QTest::keyClick(display.get(), Qt::Key_Enter);
    qDebug() << "1: " << display->text();
    if (display->text().toStdString().substr(0, 10) != "3.14159265") throw(runtime_error("1: key click Key_Enter"));

    QTest::mouseClick(buttons["M_PI"].get(), Qt::LeftButton);
    qDebug() << "2: " << display->text();
    if (display->text() != QString("M_PI")) throw(runtime_error("2: mouse click \"M_PI\""));

    QTest::mouseClick(buttons["ENTER"].get(), Qt::LeftButton);
    qDebug() << "3: " << display->text();
    if (display->text().toStdString().substr(0, 10) != "3.14159265") throw(runtime_error("3: mouse click ENTER"));

    QTest::keyClicks(display.get(), "4*(4*atan(1/5) - atan(1/239))");
    qDebug() << "4: " << display->text();
    if (display->text() != QString("4*(4*atan(1/5) - atan(1/239))")) throw(runtime_error("4: key clicks"));

    QTest::mouseClick(buttons["ENTER"].get(), Qt::LeftButton);
    qDebug() << "5: " << display->text();
    if (display->text().toStdString().substr(0, 10) != "3.14159265") throw(runtime_error("5: mouse click ENTER"));
  } catch(exception &ex) {
    qDebug() << ex.what();
    qDebug() << "Test failed";
    return -1;
  }

  qDebug() << "Test passed";
  return 0;
}
#endif

//

int main_(int argc, char **argv) {
  QApplication::setDesktopSettingsAware(false);
  QApplication::setStyle("Fusion");
  QApplication app(argc, argv);
  OctCalc calc(nullptr, &app);
  app.installEventFilter(&calc);
  calc.show();
#if !defined(TEST)
  return app.exec();
#else
  return calc.doTest();
#endif
}
