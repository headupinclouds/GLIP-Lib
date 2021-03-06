/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE 			                                                         */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : ResourceLoader.hpp                                                                        */
/*     Original Date : December 28th 2012                                                                        */
/*                                                                                                               */
/*     Description   : Qt interface for loading and writing to images.                                           */
/*                     Qt interface for loading pipelines from file.					         */
/*                     Qt widget for the interface.                                                              */
/*                                                                                                               */
/* ************************************************************************************************************* */

#ifndef __GLIPLIB_IMAGE_LOADER__
#define __GLIPLIB_IMAGE_LOADER__

	// Display messages on std::cout :
	//#define __RESOURCE_LOADER_VERBOSE__

	// Use PBO or not :
	//#define __USE_PBO__

	#include "GLIPLib.hpp"
	#include <vector>
	#include <QString>
	#include <QGLWidget>
	#include <QLabel>
	#include <QVBoxLayout>
	#include <QHBoxLayout>
	#include <QGridLayout>
	#include <QPushButton>
	#include <QSpinBox>
	#include <QLineEdit>
	#include <QComboBox>
	#include <QDialog>

	#include "UniformsVarsLoaderInterface.hpp"

	// Namespaces
	using namespace Glip;
	using namespace Glip::CoreGL;
	using namespace Glip::CorePipeline;
	using namespace Glip::Modules;

	extern ImageBuffer* createImageBufferFromQImage(const QImage& qimage);
	extern QImage* createQImageFromImageBuffer(const ImageBuffer& buffer);

	class ImageLoader
	{
		private :
			std::vector<HdlTexture*> textures;
			std::vector<std::string> filenamesList;

			void cleanResources(void);
		public :
			ImageLoader(void);
			~ImageLoader(void);

			int 			loadFiles(GLenum minFilter=GL_NEAREST, GLenum magFilter=GL_NEAREST, GLenum sWrapping=GL_CLAMP, GLenum tWrapping=GL_CLAMP, int maxLevel=0, QWidget* parent=NULL);
			int  			getNumTextures(void) const;
			HdlTexture& 		texture(int id);
			const std::string&	filename(int id);

			static bool 		saveTexture(HdlTexture& texture, QWidget* parent=NULL);
			static HdlTexture* 	createTexture(const QImage& image, GLenum minFilter=GL_NEAREST, GLenum magFilter=GL_NEAREST, GLenum sWrapping=GL_CLAMP, GLenum tWrapping=GL_CLAMP, int maxLevel=0);
			static QImage		createImage(HdlTexture& texture);
	};

	class ImageLoaderOptions : public QGridLayout
	{
		Q_OBJECT

		private :
			QLabel		minFilterLabel,
					magFilterLabel,
					sWrappingLabel,
					tWrappingLabel,
					maxLevelLabel;
			QComboBox 	minFilterBox,
					magFilterBox,
					sWrappingBox,
					tWrappingBox;
			QSpinBox	maxLevelSpinBox;

		public :
			ImageLoaderOptions(QWidget* parent=NULL, GLenum minFilter=GL_NEAREST, GLenum magFilter=GL_NEAREST, GLenum sWrapping=GL_CLAMP, GLenum tWrapping=GL_CLAMP, int maxLevel=0);
			~ImageLoaderOptions(void);

			GLenum getMinFilter(void) const;
			GLenum getMagFilter(void) const;
			GLenum getSWrapping(void) const;
			GLenum getTWrapping(void) const;
			int getMaxLevel(void) const;
	};

	class ImageLoaderOptionsDialog : public QDialog
	{
		Q_OBJECT

		private :
			QPushButton		okButton,
						cancelButton;

		public :
			ImageLoaderOptions options;

			ImageLoaderOptionsDialog(QWidget* parent=NULL, GLenum minFilter=GL_NEAREST, GLenum magFilter=GL_NEAREST, GLenum sWrapping=GL_CLAMP, GLenum tWrapping=GL_CLAMP, int maxLevel=0);
			~ImageLoaderOptionsDialog(void);
	};

	class ImageLoaderInterface : public QVBoxLayout, public ImageLoader
	{
		Q_OBJECT

		private :
			QHBoxLayout	layout1,
					layout2;
			QPushButton 	loadButton,
					optionsButton,
					prev,
					next;
			QSpinBox    	currentIndex;
			QLineEdit	maxIndex;

			GLenum		minFilter,
					magFilter,
					sWrapping,
					tWrapping;
			int		maxMipmapLevel;

		private slots :
			void changeOptions(void);
			void loadImages(void);
			void updateFilename(void);
			void updateLoadToolTip(void);
			void updateFilenameToolTip(void);

		public :
			ImageLoaderInterface(QWidget* parent=NULL);
			~ImageLoaderInterface(void);

			HdlTexture& currentTexture(void);

		public slots :
			void nextImage(void);
			void previousImage(void);

		signals :
			void currentTextureChanged(void);
	};

	class SingleUniformsVarsLoaderInterface : public QWidget
	{
		Q_OBJECT

		private : 
			QVBoxLayout			layout;
			QTreeWidget			tree;
			UniformsVarsLoaderInterface*	mainItem;

		public : 
			SingleUniformsVarsLoaderInterface(Pipeline& pipeline, QWidget* parent=NULL);
			~SingleUniformsVarsLoaderInterface(void);

			void applyTo(Pipeline& pipeline, bool forceWrite=true);

		signals : 
			void modified(void);
	};

	class PipelineLoaderInterface : public QVBoxLayout
	{
		Q_OBJECT

		private :
			QString					previousFilename;
			Pipeline				*loadedPipeline;
			SingleUniformsVarsLoaderInterface	*uniformsInterface;
			QHBoxLayout 				secondaryLayout;
			QPushButton 				loadButton,
								refreshButton,
								showUniformsVarsInterfaceButton;
			QLineEdit				pipelineName;
			QComboBox				outputChoice;
			LayoutLoader				layoutLoader;

		public :
			PipelineLoaderInterface(void);
			~PipelineLoaderInterface(void);

			bool isPipelineValid(void) const;
			bool currentChoiceIsOriginal(void) const;
			int currentOutputId(void) const;
			Pipeline& pipeline(void);
			HdlTexture& currentOutput(HdlTexture& original);
			HdlTexture& output(HdlTexture& original, int id);
			LayoutLoader& loader(void);

		public slots:
			void loadPipelineDialog(void);
			void refreshPipeline(void);
			void loadPipeline(const QString& filename);
			void revokePipeline(void);
			void showUniformsVarsInterface(void);
			void updateUniforms(void);

		signals :
			void pipelineChanged(void);
			void requestComputingUpdate(void);
			void outputIndexChanged(void);
	};

#endif
