#ifndef __GLIPSTUDIO_MAIN_INTERFACE__
#define __GLIPSTUDIO_MAIN_INTERFACE__

	#include "GLIPLib.hpp"
	#include "WindowRendering.hpp"
	#include "codeEditor.hpp"
	#include "libraryInterface.hpp"
	#include <QApplication>
	#include <QWidget>

	class MainWindow : public QWidget	
	{
		Q_OBJECT

		private : 
			QVBoxLayout		layout;
			CodeEditorsPannel 	codeEditors;

		public slots :
			void refreshPipeline(void);

		public : 
			MainWindow(void);
			~MainWindow(void);
	};

	class GlipStudio : public QApplication
	{
		Q_OBJECT

		private : 
			MainWindow *mainWindow;

		public : 
			GlipStudio(int& argc, char** argv);
			~GlipStudio(void); 
	};

#endif
