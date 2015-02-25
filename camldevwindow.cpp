// camldevwindow.cpp - LemonCaml main window
// This file is part of LemonCaml - Copyright (C) 2012-2014 Corentin FERRY
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "camldevwindow.h"

CamlDevWindow::CamlDevWindow(QString wd, QWidget *parent) :
QMainWindow(parent)
{
   
   this->camlProcess = new QProcess(this);
   this->camlStarted = false;
   this->currentFile = "";
   this->cwd = wd;
   this->unsavedChanges = false;
   this->programTitle = "LemonCaml";
   /* The window title and icon */
   this->setWindowTitle(this->programTitle + " - " + "untitled");
   this->setWindowIcon(QIcon(":/progicon.png"));
   
   this->settings = new QSettings("Cocodidou", "LemonCaml");
   
   /* The main window elements : two text-areas and a splitter */
   this->split = new QSplitter(Qt::Horizontal,this);
   this->inputZone = new InputZone();
   this->inputZone->setTabStopWidth(20);
   this->inputZone->setAcceptRichText(false);
   
   this->resize(settings->value("Size/y",800).toInt(), settings->value("Size/x",600).toInt());
   this->move(settings->value("Pos/x",0).toInt(), settings->value("Pos/y",0).toInt());
   this->setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint));
   
   QString iFont = settings->value("Input/Font", "").toString();
   QFont inputFont;
   inputFont.fromString(iFont);
   this->inputZone->setFont(inputFont);
   
   this->outputZone = new QTextEdit(this);
   this->outputZone->setReadOnly(true);
   this->outputZone->setTabStopWidth(20);
   
   QString kwfileloc = settings->value("General/keywordspath", "./keywords").toString();
   
   QFile kwfile(kwfileloc);
   QStringList kwds;
   
   if(kwfile.open(QIODevice::ReadOnly | QIODevice::Text))
   {
      QTextStream kstream(&kwfile);
      QString st = kstream.readLine(256);
      while(st != "")
      {
         kwds << st;
         st = kstream.readLine(256);
      }
      kwfile.close();
   }
   else
   {
      QMessageBox::warning(this, "Warning", "Unable to open the keywords file. There will likely be no syntax highlighting.");
   }
   this->hilit = new highlighter(inputZone->document(), &kwds, this->settings);
   
   
   
   QString oFont = settings->value("Output/Font", "").toString();
   QFont outputFont;
   outputFont.fromString(oFont);
   this->outputZone->setFont(outputFont);
   
   this->setCentralWidget(split);
   
   
   split->addWidget(this->inputZone);
   split->addWidget(this->outputZone);
   split->showMaximized();
   
   /* the printer*/
   this->printer = new QPrinter(QPrinter::HighResolution);
   
   /* The actions */
   this->actionNew = new QAction("New",this);
   this->actionNew->setIcon(QIcon(":/new.png"));
   this->actionNew->setShortcut(QKeySequence(QKeySequence::New));
   this->actionOpen = new QAction("Open",this);
   this->actionOpen->setIcon(QIcon(":/open.png"));
   this->actionOpen->setShortcut(QKeySequence(QKeySequence::Open));
   this->actionSaveAs = new QAction("Save As",this);
   this->actionSaveAs->setIcon(QIcon(":/saveas.png"));
   this->actionSaveAs->setShortcut(QKeySequence(QKeySequence::SaveAs));
   this->actionSave = new QAction("Save",this);
   this->actionSave->setIcon(QIcon(":/save.png"));
   this->actionSave->setShortcut(QKeySequence(QKeySequence::Save));
   this->actionAutoIndent = new QAction("Auto-indent code",this);
   this->actionPrint = new QAction("Print",this);
   this->actionPrint->setIcon(QIcon(":/print.png"));
   this->actionPrint->setShortcut(QKeySequence(QKeySequence::Print));
   this->actionClearOutput = new QAction("Clear output",this);
   this->actionQuit = new QAction("Quit",this);
   this->actionQuit->setIcon(QIcon(":/exit.png"));
   this->actionQuit->setShortcut(QKeySequence(QKeySequence::Quit));
   
   this->actionUndo = new QAction("Undo",this);
   this->actionUndo->setIcon(QIcon(":/undo.png"));
   this->actionUndo->setShortcut(QKeySequence(QKeySequence::Undo));
   this->actionRedo = new QAction("Redo",this);
   this->actionRedo->setIcon(QIcon(":/redo.png"));
   this->actionRedo->setShortcut(QKeySequence(QKeySequence::Redo));
   this->actionDelete = new QAction("Delete",this);
   this->actionChangeInputFont = new QAction("Change Input Font",this);
   this->actionChangeOutputFont = new QAction("Change Output Font",this);
   
   this->actionSendCaml = new QAction("Send Code to Caml",this);
   this->actionSendCaml->setIcon(QIcon(":/sendcaml.png"));
   this->actionSendCaml->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return));
   this->actionInterruptCaml = new QAction("Interrupt Caml",this);
   this->actionInterruptCaml->setIcon(QIcon(":/interrupt.png"));
   this->actionStopCaml = new QAction("Stop Caml",this);
   this->actionStopCaml->setIcon(QIcon(":/stopcaml.png"));
   this->actionShowSettings = new QAction("Settings",this);
   this->actionShowSettings->setShortcut(QKeySequence(QKeySequence::Preferences));
   
   this->actionAbout = new QAction("About LemonCaml...",this);
   this->actionAboutQt = new QAction("About Qt...",this);
   
   this->actionHighlightEnable = new QAction("Enable syntax highlighting", this);
   this->actionHighlightEnable->setIcon(QIcon(":/highlight.png"));
   this->actionHighlightEnable->setCheckable(true);
   this->actionHighlightEnable->setChecked(true);
   
   this->actionZoomIn = new QAction("Zoom in", this);
   this->actionZoomIn->setShortcut(QKeySequence(QKeySequence::ZoomIn));
   this->actionZoomOut = new QAction("Zoom out", this);
   this->actionZoomOut->setShortcut(QKeySequence(QKeySequence::ZoomOut));
   
   /* The toolbar */
   this->toolbar = new QToolBar("Tools",this);
   this->toolbar->addAction(actionNew);
   this->toolbar->addAction(actionOpen);
   this->toolbar->addAction(actionSave);
   this->toolbar->addAction(actionSaveAs);
   this->toolbar->addAction(actionPrint);
   this->toolbar->addSeparator();
   this->toolbar->addAction(actionUndo);
   this->toolbar->addAction(actionRedo);
   this->toolbar->addSeparator();
   this->toolbar->addAction(actionSendCaml);
   this->toolbar->addAction(actionInterruptCaml);
   this->toolbar->addAction(actionStopCaml);
   this->toolbar->addAction(actionHighlightEnable);
   this->addToolBar(this->toolbar);
   
   /* The menubar */
   this->menuFile = this->menuBar()->addMenu("File");
   this->menuFile->addAction(actionNew);
   this->menuFile->addAction(actionOpen);
   this->menuRecent = this->menuFile->addMenu("Recent files");
   this->menuFile->addAction(actionSave);
   this->menuFile->addAction(actionSaveAs);
   this->menuFile->addAction(actionPrint);
   this->menuFile->addAction(actionQuit);
   
   this->menuEdit = this->menuBar()->addMenu("Edit");
   this->menuEdit->addAction(actionUndo);
   this->menuEdit->addAction(actionRedo);
   this->menuEdit->addAction(actionDelete);
   this->menuEdit->addSeparator();
   this->menuEdit->addAction(actionAutoIndent);
   this->menuEdit->addAction(actionClearOutput);
   this->menuEdit->addAction(actionHighlightEnable);
   this->menuEdit->addAction(actionChangeInputFont);
   this->menuEdit->addAction(actionChangeOutputFont);
   this->menuEdit->addAction(actionZoomIn);
   this->menuEdit->addAction(actionZoomOut);
   
   this->menuCaml = this->menuBar()->addMenu("Caml");
   this->menuCaml->addAction(actionSendCaml);
   this->menuCaml->addAction(actionInterruptCaml);
   this->menuCaml->addAction(actionStopCaml);
   this->menuCaml->addAction(actionShowSettings);
   
   this->menuHelp = this->menuBar()->addMenu("Help");
   this->menuHelp->addAction(actionAbout);
   this->menuHelp->addAction(actionAboutQt);
   
   /* Connections */
   connect(actionSendCaml,SIGNAL(triggered()),this,SLOT(sendCaml()));
   connect(camlProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(readCaml()));
   connect(camlProcess,SIGNAL(readyReadStandardError()),this,SLOT(readCamlErrors()));
   connect(camlProcess,SIGNAL(stateChanged(QProcess::ProcessState)),this,SLOT(updateCamlStatus(QProcess::ProcessState)));
   connect(actionStopCaml,SIGNAL(triggered()),this,SLOT(stopCaml()));
   connect(camlProcess,SIGNAL(started()),this,SLOT(camlOK()));
   connect(actionInterruptCaml,SIGNAL(triggered()),this,SLOT(interruptCaml()));
   
   
   connect(actionSave,SIGNAL(triggered()),this,SLOT(save()));
   connect(actionSaveAs,SIGNAL(triggered()),this,SLOT(saveAs()));
   connect(actionOpen,SIGNAL(triggered()),this,SLOT(open()));
   connect(inputZone,SIGNAL(textChanged()),this,SLOT(textChanged()));
   connect(actionNew,SIGNAL(triggered()),this,SLOT(newFile()));
   connect(actionClearOutput,SIGNAL(triggered()),this->outputZone,SLOT(clear()));
   connect(actionChangeInputFont,SIGNAL(triggered()),this,SLOT(changeInputFont()));
   connect(actionChangeOutputFont,SIGNAL(triggered()),this,SLOT(changeOutputFont()));
   connect(actionQuit,SIGNAL(triggered()),this,SLOT(close()));
   connect(actionPrint,SIGNAL(triggered()),this,SLOT(print()));
   connect(actionUndo,SIGNAL(triggered()),this->inputZone,SLOT(undo()));
   connect(actionRedo,SIGNAL(triggered()),this->inputZone,SLOT(redo()));
   connect(actionDelete,SIGNAL(triggered()),this->inputZone,SLOT(paste()));
   connect(actionShowSettings,SIGNAL(triggered()),this,SLOT(showSettings()));
   connect(actionHighlightEnable,SIGNAL(toggled(bool)),this,SLOT(toggleHighlightOn(bool)));
   connect(actionAutoIndent,SIGNAL(triggered()),this,SLOT(autoIndentCode()));
   
   connect(actionZoomIn,SIGNAL(triggered()),this,SLOT(zoomIn()));
   connect(actionZoomOut,SIGNAL(triggered()),this,SLOT(zoomOut()));
   
   
   connect(actionAbout,SIGNAL(triggered()),this,SLOT(about()));
   connect(actionAboutQt,SIGNAL(triggered()),this,SLOT(aboutQt()));
   
   this->generateRecentMenu();
   this->populateRecent();
   
   this->highlightTriggered = false;
   fillIndentWords(&indentWords);
   
   //Draw trees?
   this->drawTrees = (settings->value("General/drawTrees",0).toInt() == 1)?true:false;
   this->graphCount = 0;
   
   this->startCamlProcess();
}

CamlDevWindow::~CamlDevWindow()
{
   camlProcess->close();
   
}

bool CamlDevWindow::startCamlProcess()
{
   /* Start the Caml process */
   #ifdef WIN32
   QString args = settings->value("General/camlArgs", "-stdlib ./caml/lib").toString();
   //camlProcess->setWorkingDirectory(camlLibPath);
   QString camlProcessPath = settings->value("General/camlPath", "./caml/CamlLightToplevel.exe").toString();
   camlProcess->start(camlProcessPath + " " + args);
   #else
   QString args = settings->value("General/camlArgs", "-stdlib ./caml/lib").toString();
   //camlProcess->setWorkingDirectory(camlLibPath);
   QString camlProcessPath = settings->value("General/camlPath", "./caml/CamlLightToplevel").toString();
   camlProcess->start(camlProcessPath + " " + args);
   #endif
   return (camlProcess->state() == QProcess::Starting || camlProcess->state() == QProcess::Running);
}

void CamlDevWindow::updateCamlStatus(QProcess::ProcessState newState)
{
   switch(newState)
   {
      case QProcess::NotRunning:
      case QProcess::Starting:
         if(camlStarted)
         {
            //this->outputZone->setTextColor(this->palette().color(QPalette::WindowText));
            //this->outputZone->append("Caml Stopped\n-----------\n\n");
            appendOutput("\nCaml Stopped\n-----------\n\n",this->palette().color(QPalette::WindowText));
            camlStarted = false;
         }
         break;
         
      case QProcess::Running:
         camlStarted = true;
         break;
         
      default:
         break;
         
   }
}

void CamlDevWindow::sendCaml()
{
   bool prevCaml = false; //hack to let caml print "Caml Light v0.75..." before outputting the command
   if(!camlStarted)
   {
      prevCaml = true;
      startCamlProcess();
      camlProcess->waitForStarted();
      if(!camlStarted)
      {
         QMessageBox::warning(this,"Warning","Unable to start Caml toplevel!! Please go to Caml -> Settings to set its path.");
      }
      
   }
   int curPos = 0;
   int startPos = 0;
   int endPos = 0;
   QTextCursor cursor = inputZone->textCursor();
   QString text = inputZone->toPlainText();
   if(!cursor.hasSelection())
   {
      curPos = cursor.position();

      //QString text = removeComments(text_r);
      startPos = (text.lastIndexOf(";;",curPos)) + 2;
      if(startPos == 1)
      {
         startPos = 0;
      }
      
      endPos = (text.indexOf(";;",curPos)) + 2;
      if(endPos == 1) endPos = text.length();
      if(curPos == text.length()) //avoid sending the whole text
      {
         startPos = (text.lastIndexOf(";;",curPos - 2)) + 2;
         if(startPos == 1) startPos = 0;
      }
   }
   else
   {
      startPos = cursor.selectionStart();
      curPos = startPos;
      endPos = cursor.selectionEnd();
   }
   QString toWrite = text.mid(startPos,endPos - startPos)  + "\n\0";
   toWrite = removeComments(toWrite);
   toWrite = removeUnusedLineBreaks(toWrite,true);
   
   if(prevCaml) {
      
      readCaml();
   }
   appendOutput(toWrite,Qt::blue);
   
   camlProcess->write(toWrite.toLatin1());
   
   int nextCurPos = text.indexOf(";;",curPos) + 2;
   if(nextCurPos == 1){ nextCurPos = text.length();}
   
   cursor.setPosition(nextCurPos,QTextCursor::MoveAnchor);
   inputZone->setTextCursor(cursor);
}

void CamlDevWindow::readCamlErrors()
{
   QString stdErr = camlProcess->readAllStandardError();
   stdErr = removeUnusedLineBreaks(stdErr,false);
   if(stdErr != "") appendOutput(stdErr,Qt::red);
   
}

void CamlDevWindow::readCaml()
{
   
   QString stdOut = camlProcess->readAllStandardOutput();
   stdOut = removeUnusedLineBreaks(stdOut,false);
   if(drawTrees)
   {
      while(stdOut.indexOf("--LemonCamlCommand--") != -1)
      {
         int j = stdOut.indexOf("--LemonCamlCommand--"); //20
         int p = stdOut.indexOf("--EndLemonCamlCommand--"); //23
         if(p == -1)
         {
            appendOutput("---LemonCaml error--- Unterminated command: not interpreted\n", Qt::red);
            stdOut = stdOut.mid(j + 20);
         }
         else
         {
            appendOutput(stdOut.left(j),this->palette().color(QPalette::WindowText));
            QString cmd = stdOut.mid(j + 20, (p - j - 20));
            QStringList cmdlist = parseBlockCommand(cmd);

            processCommandList(&cmdlist);
            stdOut = stdOut.mid(p + 23);
         }
      }
      while(stdOut.indexOf("--LemonTree--") != -1) //TODO: change this!!!! never leave such an absolute type name here; ask the user, instead.
      {
         int j = stdOut.indexOf("--LemonTree--"); // 17
         int p = stdOut.indexOf("--EndLemonTree--"); //20
         if(p == -1)
         {
            appendOutput("---LemonCaml error--- Unterminated tree: not drawn\n", Qt::red);
            stdOut = stdOut.mid(j + 17);  
         }
         else
         {
            appendOutput(stdOut.left(j),this->palette().color(QPalette::WindowText));
            QString arb = stdOut.mid(j + 13, (p - j - 13));
            int k = arb.indexOf('(');
            if(k != -1)
            {
               QString arbString = arb.mid(k);
               treeParser* tp = new treeParser();
               QImage* img = tp->parseTree(arbString);
               QTextCursor cursor = outputZone->textCursor();
               cursor.insertImage((*img), QString(this->graphCount));
               outputZone->insertPlainText("\n");
               this->graphCount++;
            }
            stdOut = stdOut.mid(p + 16);
         }
         
      }
   }
   if(stdOut != "") appendOutput(stdOut,this->palette().color(QPalette::WindowText));
   
}

void CamlDevWindow::camlOK()
{
   this->camlStarted = true;
}

void CamlDevWindow::stopCaml()
{
   camlProcess->close();
}

void CamlDevWindow::interruptCaml()
{
   if(camlProcess->state() == QProcess::Running)
   {
#ifndef WIN32
      kill(camlProcess->pid(), SIGINT);
#else
   
appendOutput("\n\nWARNING: The \"Interrupt Caml\" command is not available under Windows.\n\
It will not be available until QProcess handles process group IDs (let's say, never). More info at:\n\
http://stackoverflow.com/questions/22255851/sending-ctrlc-event-to-a-process-launched-using-qprocess-on-windows\n\n\
Please use \"Stop Caml\" instead, then send your code back to Caml.\n", Qt::red);
#endif

   }
}



bool CamlDevWindow::saveAs()
{
   QString fileName = QFileDialog::getSaveFileName(this,"Save As...","","Caml source files (*.ml *.mli);;Text files (*.txt);;All files(*)");
   if(fileName.isEmpty())
   {
      return false;
   }
   currentFile = fileName;
   bool success = saveFile(currentFile);
   if(success){
      updateRecent();
   }
   return success;
   
}

bool CamlDevWindow::save()
{
   if(currentFile.isEmpty()) return saveAs();
   else return saveFile(currentFile);
}

bool CamlDevWindow::saveFile(QString file)
{
   QFile f(file);
   if(!f.open(QFile::WriteOnly))
   {
      QMessageBox::warning(this,"Warning","Unable to save file !!");
      return false;
   }
   //QTextCodec *codec = QTextCodec::codecForName("UTF-8");
   QString output = inputZone->toPlainText(); //codec->fromUnicode(inputZone->toPlainText());
   
   f.write(output.toUtf8());
   f.close();
   this->setWindowTitle(this->programTitle + " - " + f.fileName());
   this->unsavedChanges = false;
   return true;
}

void CamlDevWindow::open()
{
   QString fileName = QFileDialog::getOpenFileName(this,"Open","","Caml source files (*.ml *.mli);;Text files (*.txt);;All files(*)");
         
   if(!fileName.isEmpty())
   {
      //currentFile = fileName;
      openFile(fileName);
   }
   
}

void CamlDevWindow::openFile(QString file)
{
   if(!exitCurrentFile()) return;
   this->unsavedChanges = false;
   QFile f(file);
   if(!f.open(QFile::ReadOnly))
   {
      QMessageBox::warning(this,"Warning","Unable to open file " + file + "!");
      return;
   }
   //QString curFile = currentFile;
   
   this->newFile(); //clear everything
   currentFile = file; //restore filename
   updateRecent();
   QTextCodec *codec = QTextCodec::codecForName("UTF-8");
   inputZone->clear();
   inputZone->setText(codec->toUnicode(f.readAll()));
   this->setWindowTitle(this->programTitle + " - " + f.fileName());
   this->unsavedChanges = false;
   QTextCursor cursor = inputZone->textCursor();
   cursor.setPosition(0,QTextCursor::MoveAnchor);
   inputZone->setTextCursor(cursor);
   
}

void CamlDevWindow::appendOutput(QString str, QColor color)
{
   QTextCursor tc = outputZone->textCursor();
   tc.setPosition(outputZone->toPlainText().length());
   outputZone->setTextCursor(tc);
   outputZone->setTextColor(color);
   outputZone->insertPlainText(str);
   tc.setPosition(outputZone->toPlainText().length());
   outputZone->setTextCursor(tc);
}



void CamlDevWindow::textChanged()
{
   if(!this->unsavedChanges && !highlightTriggered)
   {
      this->unsavedChanges = true;
      this->setWindowTitle(this->windowTitle() + " (*)");
   }
   if(highlightTriggered) highlightTriggered = false;
}

void CamlDevWindow::newFile()
{
   if(!exitCurrentFile()) return;
   this->stopCaml();
   this->inputZone->clear();
   this->treevars.clear();
   this->treevalues.clear();
   this->unsavedChanges = false;
   this->currentFile = "";
   this->outputZone->clear();
   this->graphCount = 0;
   this->setWindowTitle(this->programTitle + " - " + "untitled");
   while(camlProcess->state() != QProcess::NotRunning)
   {
      camlProcess->close();
   }
   this->startCamlProcess();
}



bool CamlDevWindow::exitCurrentFile()
{
   if(!unsavedChanges) return true;
   else
   {
      int btn = QMessageBox::question(this,"Save changes before closing?","Your changes have not been saved! Would you like to do that now?",QMessageBox::Save,QMessageBox::Discard,QMessageBox::Cancel);
      bool ret = false;
      switch(btn)
      {
         
         case QMessageBox::Save:
            ret = this->save(); break;
         case QMessageBox::Discard:
            ret = true; break;
         case QMessageBox::Cancel:
            ret = false; break;
            
      }
      return ret;
   }
   
}

void CamlDevWindow::closeEvent(QCloseEvent *event)
{
   if(exitCurrentFile()) event->accept();
   else event->ignore();
}

void CamlDevWindow::print()
{
   QPrintDialog dlg(printer, this);
   //dlg.open(this, SLOT(doPrint()));
   if (dlg.exec() == QDialog::Accepted) {
      doPrint();
   }
}

void CamlDevWindow::changeInputFont()
{
   this->inputZone->setFont(QFontDialog::getFont(0, inputZone->font()));
   QFont fnt = inputZone->font();
   settings->setValue("Input/Font", fnt.toString());
}

void CamlDevWindow::changeOutputFont()
{
   this->outputZone->setFont(QFontDialog::getFont(0, outputZone->font()));
   QFont fnt = outputZone->font();
   settings->setValue("Output/Font", fnt.toString());
}

void CamlDevWindow::doPrint()
{
   this->inputZone->print(printer); 
}

void CamlDevWindow::showSettings()
{
   CamlDevSettings s(this, this->settings);
   s.exec();
   this->drawTrees = (settings->value("General/drawTrees",0).toInt() == 1)?true:false;
   this->generateRecentMenu();
   this->populateRecent();
   
   this->highlightTriggered = true; //do not account this setting change for an actual text change
   hilit->setDocument(NULL);
   this->hilit->updateColorSettings();
   
   this->highlightTriggered = true;
   hilit->setDocument(this->actionHighlightEnable->isChecked() ? inputZone->document() : 0);
   
}

void CamlDevWindow::zoomIn()
{
   inputZone->zoomIn();
   outputZone->zoomIn();
}

void CamlDevWindow::zoomOut()
{
   inputZone->zoomOut();
   outputZone->zoomOut();
}

void CamlDevWindow::about()
{
   QMessageBox::about(this, "About LemonCaml", "<b>This is LemonCaml, (c) 2012-2014 Corentin FERRY.</b><br />\
   LemonCaml is a simple, basic-featured Caml development environment.<br />\
   It is released under the GPLv3 license; see COPYING for details.");
   
}

void CamlDevWindow::aboutQt()
{
   QMessageBox::aboutQt(this, "About Qt");
}

void CamlDevWindow::resizeEvent(QResizeEvent *event)
{
   QSize sz = event->size();
   settings->setValue("Size/y", sz.width());
   settings->setValue("Size/x", sz.height());
}

void CamlDevWindow::moveEvent(QMoveEvent *event)
{
   QPoint p = event->pos();
   settings->setValue("Pos/y", p.y());
   settings->setValue("Pos/x", p.x());
}

void CamlDevWindow::generateRecentMenu()
{
   
   if(this->recent != NULL)
   {
      for(int i = 0; i < numRecentFiles; i++)
      {
         delete recent[i];
         
      }
      delete[] this->recent;
      delete[] this->recentFiles;
   }
   
   numRecentFiles = settings->value("Recent/number", 5).toInt();
   this->recent = new QAction*[numRecentFiles];
   this->recentFiles = new QString[numRecentFiles];
   for(int i = 0; i < numRecentFiles; i++)
   {
      recent[i] = new QAction("(empty)", this);
      this->menuRecent->addAction(recent[i]);
   }
}

void CamlDevWindow::populateRecent()
{
   //int fileCount = settings->value("Recent/number", 5).toInt();
   for(int i = 0; i < numRecentFiles; i++)
   {
      QString loc = settings->value("Recent/file" + QString(i), "").toString();
      if(loc != "")
      {
         recentFiles[i] = loc;
         recent[i]->setText(loc);
         connect(recent[i], SIGNAL(triggered()), this, SLOT(openRecent()));
      }
      else recentFiles[i] = "";
   }
}

void CamlDevWindow::updateRecent()
{
   bool found = false;
   for(int i = 0; i < numRecentFiles; i++)
   {
      if(currentFile == recentFiles[i])
      {
         int cpt = i;
         while (cpt > 0)
         {
            recentFiles[cpt] = recentFiles[cpt - 1];
            cpt--;
         }
         recentFiles[0] = currentFile;
         found = true;
      }
   }
   
   if(!found)
   {
      int cpt = numRecentFiles - 1;
      while(cpt > 0)
      {
         recentFiles[cpt] = recentFiles[cpt - 1];
         cpt--;
      }
      recentFiles[0] = currentFile;
   }
   
   //then, update the settings
   for(int i = 0; i < numRecentFiles; i++)
   {
      if(recentFiles[i] != "") recent[i]->setText(recentFiles[i]);
      else recent[i]->setText("(empty)");
      settings->setValue("Recent/file" + QString(i), recentFiles[i]);
   }
   
   return;
}

void CamlDevWindow::openRecent()
{
   if(sender())
   {
      QAction* snd = (QAction*) sender();
      if(snd->text() != "(empty)")
         openFile(snd->text());
   }
}

void CamlDevWindow::toggleHighlightOn(bool doHighlight)
{
   highlightTriggered = true;
   hilit->setDocument(doHighlight ? inputZone->document() : 0);
}

void CamlDevWindow::processCommandList(QStringList *commands)
{
   //appendOutput("---Begin LemonCaml processing---\n", this->palette().color(QPalette::WindowText));
   while(commands->count() > 0)
   {
      if(commands->at(0) == "SetupPrinter")
         processSetupPrinter(commands);
      else if(commands->at(0) == "SubstituteTree")
         processSubstituteTree(commands);
      else if(commands->at(0) == "RegisterTreeType")
         processRegisterTreeType(commands);
      else if(commands->at(0) == "SendCaml" && commands->count() > 1)
      {
         commands->removeFirst();
         QString cm = commands->takeFirst();
         //appendOutput(cm, Qt::blue);
         camlProcess->write(cm.toLatin1());
      }
      else
      {
         appendOutput("---LemonCaml error--- Unknown command: " + commands->takeFirst() + "\n", Qt::red);
      }

   }
   //appendOutput("---End LemonCaml processing---\n", this->palette().color(QPalette::WindowText));
}

void CamlDevWindow::processSetupPrinter(QStringList *commands)
{
   if(commands->at(0) == "SetupPrinter") //checking: we might have been called from elsewhere...
   {
      commands->removeFirst(); //drop the 1st elt
      if(commands->count() > 0)
      {
         QString built = "#open \"format\";;\n \
         install_printer \"" + commands->takeFirst() + "\";;\n";
         //appendOutput(built, Qt::blue);
         camlProcess->write(built.toLatin1());
      }
   }
}

void CamlDevWindow::processSubstituteTree(QStringList *commands)
{
   //Syntax: SubstituteTree [var]
   if(commands->at(0) == "SubstituteTree") //checking: we might have been called from elsewhere...
   {
      commands->removeFirst(); //drop the 1st elt
      QString arg = commands->takeFirst(); //take the 2nd element (our argument)
      int i = 0;
      bool found = false;
      QString subs = "";
      while(i < treevars.count() && !found)
      {
         if(treevars[i] == arg)
         {
            found = true;
            subs = treevalues[i];
         }
         i++;
      }
      if(!found)
         appendOutput("---LemonCaml error--- Unknown variable: " + arg, Qt::red);
      
      //appendOutput(subs, Qt::blue);
      camlProcess->write(subs.toLatin1());

   }
}

void CamlDevWindow::processRegisterTreeType(QStringList *commands)
{
   //Syntax: RegisterTreeType [type] [var=value]
   if(commands->at(0) == "RegisterTreeType" && commands->count() >= 3) 
   {
      commands->removeFirst(); //drop the 1st elt
      QString treetype = commands->takeFirst();
      
      //clear the existing vars
      this->treevars.clear();
      this->treevalues.clear();
      
      QString vars = commands->takeFirst();
      QStringList varsplit = vars.split(";", QString::SkipEmptyParts);
      for(int i = 0; i < varsplit.count(); i++)
      {
         QStringList reg = varsplit[i].split("=", QString::KeepEmptyParts);
         if(reg[0] != "" && reg.count() == 2 && reg[1] != "")
         {
            this->treevars << reg[0];
            this->treevalues << reg[1];
         }
      }
      QString MLLoc = settings->value("General/treeModelsPath","./gentree/").toString();
      this->autoLoadML(MLLoc + treetype + ".ml"); //load the ML file that auto-registers the tree type
   }
}

void CamlDevWindow::autoLoadML(QString location)
{
   QString built = "include \"" + location + "\";;\n";
   //appendOutput(built, Qt::blue);
   camlProcess->write(built.toLatin1());
}

void CamlDevWindow::autoIndentCode()
{
   QString code = inputZone->toPlainText();
   QString result = removeIndent(code);
   QString indentedCode = indentCode(result, &indentWords);
   
   inputZone->selectAll();
   
   inputZone->insertPlainText(indentedCode);
}