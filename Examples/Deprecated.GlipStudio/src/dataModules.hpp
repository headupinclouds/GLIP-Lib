#ifndef __GLIPSTUDIO_MODULES_DATA__
#define __GLIPSTUDIO_MODULES_DATA__

	/*
		This code is the backbone of the program. It links and manages the dialog between modules.
	*/

	// Includes : 
	#include "GLIPLib.hpp"
	#include "glSceneWidget.hpp"
	#include <QObject>

	#include <QtGlobal>
	#if QT_VERSION >= 0x050000
		#include <QtWidgets>
	#else
		#include <QtGui>
	#endif

	using namespace Glip;
	using namespace Glip::CoreGL;
	using namespace Glip::CorePipeline;
	using namespace Glip::Modules;

	// Prototypes : 
	class Module;
	class ControlModule;

	class Module : public QWidget
	{
		Q_OBJECT

		private : 
			ControlModule* masterModule;

			friend class ControlModule;
			
		protected : 
			Module(ControlModule& _masterModule, QWidget* parent = NULL);
		
			// Replace these in order to tune the reaction of the module in preparation of some event : 
			// e.g. : can the pipeline be destroyed, return true if there the user confirm by saving some data or discard current data.
			virtual bool pipelineCanBeCreated(void);
			virtual bool pipelineCanBeCompiled(void);
			virtual bool pipelineCanBeComputed(void);
			virtual bool pipelineInputsCanBeModified(void);
			virtual bool textureInputCanBeReleased(int portID, int recordID);
			virtual bool pipelineUniformsCanBeModified(void);
			virtual bool pipelineCanBeDestroyed(void);
			virtual bool canBeClosed(void);

			// Call these in order to perform actions on the pipeline : 
			bool requirePipelineCreation(const std::string& code);
			bool requirePipelineComputation(void);
			bool registerInputTexture(int recordID, int portID);
			void unregisterInputTexture(int recordID);
			void releaseInputPort(int portID);				// Force the input port to be released, even if this Module is not its owner.
			bool requirePrepareToPipelineUniformsModification(void);
			bool requirePipelineDestruction(void);

			// Special functions : 
			ViewLink* getViewLink(void);
			ViewManager* getViewManager(void);

			virtual void preparePipelineLoading(LayoutLoader& loader, const LayoutLoader::PipelineScriptElements& infos);
			virtual bool isValidTexture(int recordID);
			virtual HdlTexture& getTexture(int recordID);
			virtual const __ReadOnly_HdlTextureFormat& getTextureFormat(int portID) const;
			virtual void giveTextureInformation(int recordID, std::string& name);
			bool isListedAsPipelineInput(int recordID, int* portID = NULL) const;
			bool isUsedAsPipelineInput(int recordID, int* portID = NULL) const;
			bool isInputPortOwner(int portID, int* recordID = NULL) const;

		public : 
			virtual ~Module(void);

			// Generic tools : 
			bool pipelineExists(void) const;
			bool lastComputationWasSuccessful(void) const;
			bool isInputValid(int portID);
			HdlTexture& inputTexture(int portID);
			const __ReadOnly_HdlTextureFormat& inputTextureFormat(int portID) const;
			void getInputTextureInformation(int portID, std::string& name);
			const std::string& getPipelineCode(void) const;
			const Pipeline& pipeline(void) const;
			Pipeline& pipeline(void);
			LayoutLoader::PipelineScriptElements scan(const std::string& code);

		protected slots :
			// These will be called upon external modifications, in order for the module to update itself :
			virtual void pipelineWasCreated(void);
			virtual void pipelineCompilationFailed(const Exception& e);
			virtual void pipelineWasComputed(void);
			virtual void pipelineComputationFailed(const Exception& e);
			virtual void pipelineInputWasModified(int portID);
			virtual void pipelineInputWasReleased(int portID);
			virtual void pipelineInputFromThisModuleWasReleased(int portID, int recordID);
			virtual void pipelineUniformsWereModified(void);
			virtual void pipelineWasDestroyed(void);
			virtual void mouseParametersWereUpdated(const GLSceneWidget::MouseData& data);

		signals : 
			// These must be called after performing some actions on the pipeline :
			// (note that for creation and destruction, the operation is automatically carried by the request)
			void pipelineUniformModification(void);
	};

	class ControlModule : public QMainWindow
	{
		Q_OBJECT

		public :
			static const int 		maxNumInputs;

		private : 
			bool				lastComputationSucceeded;
			std::string			pipelineCode;
			Pipeline*			pipelinePtr;
			LayoutLoader			pipelineLoader;

			std::vector<int>		inputTextureRecordIDs;
			std::vector<Module*>		inputTextureOwners;

			std::vector<Module*>		clients;
			Module*				displayClient;

			bool pipelineCompilation(void);
			bool pipelineComputation(void);

		protected : 
			GLSceneWidgetContainer display;

			ControlModule(void);

		public : 			
			virtual ~ControlModule(void);

			// Tools : 
			void addClient(Module* m);
			void removeClient(Module* m);
			ViewLink* getViewLink(void);
			ViewManager* getViewManager(void);
			bool pipelineExists(void) const;
			bool lastComputationWasSuccessful(void) const;
			bool isInputValid(int portID);
			HdlTexture& inputTexture(int portID);
			const __ReadOnly_HdlTextureFormat& inputTextureFormat(int portID) const;
			void getInputTextureInformation(int portID, std::string& name);
			bool isListedAsPipelineInput(const Module* m, int recordID, int* portID = NULL) const;
			bool isUsedAsPipelineInput(const Module* m, int recordID, int* portID = NULL) const;
			bool isInputPortOwner(const Module* m, int portID, int* recordID = NULL) const;
			const std::string& getPipelineCode(void) const;
			const Pipeline& pipeline(void) const;
			Pipeline& pipeline(void);
			const LayoutLoader& loader(void) const;
			LayoutLoader::PipelineScriptElements scan(const std::string& code);
		
			// Distribution of the requirements : 
			bool requirePipelineCreation(const std::string& code);
			bool requirePipelineComputation(void);
			bool registerInputTexture(Module* m, int recordID, int portID);
			void unregisterInputTexture(Module* m, int recordID);
			void releaseInputPort(int portID);
			bool requirePrepareToPipelineUniformsModification(void);
			bool requirePipelineDestruction(bool keepCode=false);
			bool requireClose(void);

		public slots : 
			// Receive information of action performed by modules and redistribute to all modules : 
			void pipelineUniformModification(void);

		signals :
			void pipelineWasCreated(void);
			void pipelineCompilationFailed(const Exception& e);
			void pipelineWasComputed(void);
			void pipelineComputationFailed(const Exception& e);
			void pipelineInputWasModified(int portID);
			void pipelineInputWasReleased(int portID);
			void pipelineUniformsWereModified(void);
			void pipelineWasDestroyed(void);
	};

#endif
