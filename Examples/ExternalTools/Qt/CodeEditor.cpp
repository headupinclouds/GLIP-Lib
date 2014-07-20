/* ************************************************************************************************************* */
/*                                                                                                               */
/*     GLIP-LIB                                                                                                  */
/*     OpenGL Image Processing LIBrary                                                                           */
/*                                                                                                               */
/*     Author        : R. KERVICHE 			                                                         */
/*     LICENSE       : MIT License                                                                               */
/*     Website       : http://sourceforge.net/projects/glip-lib/                                                 */
/*                                                                                                               */
/*     File          : CodeEditor.hpp                                                                            */
/*     Original Date : July 6th 2014                                                                             */
/*                                                                                                               */
/*     Description   : Qt interface for GLIPLIB pipeline source code editor.                                     */
/*                                                                                                               */
/* ************************************************************************************************************* */

#include "CodeEditor.hpp"
#include "GLSLKeywords.hpp"

using namespace QGED;

// Highlighter
	Highlighter::Highlighter(QTextDocument *parent)
	 : QSyntaxHighlighter(parent), highlightEnabled(true)
	{
		HighlightingRule rule;

		// GLSL Keywords :
			QStringList glslkeywordPatterns;

			for(int i=0; i<GLSLLanguage::KW_END; i++)
			{
				std::string str = std::string("\\b") + GLSLLanguage::GLSLKeywords[i] + "\\b";
				glslkeywordPatterns << str.c_str();
			}

			foreach(const QString& pattern, glslkeywordPatterns)
			{
				rule.pattern = QRegExp(pattern);
				rule.format = &glslkeywordFormat;
				highlightingRules.append(rule);
			}

		// GLSL Functions : 
			QStringList glslfunctionPatterns;

			for(int i=0; i<GLSLLanguage::FN_END; i++)
			{
				std::string str = std::string("\\b") + GLSLLanguage::GLSLFunctions[i] + "\\b";
				glslfunctionPatterns << str.c_str();
			}

			foreach(const QString& pattern, glslfunctionPatterns)
			{
				rule.pattern = QRegExp(pattern);
				rule.format = &glslfunctionFormat;
				highlightingRules.append(rule);
			}

		// GLIP LayoutLoader Keywords : 
			QStringList glipllkeywordPatterns;

			for(int i=0; i<Glip::Modules::LL_NumKeywords; i++)
			{
				std::string str = std::string("\\b") + LayoutLoader::getKeyword(static_cast<Glip::Modules::LayoutLoaderKeyword>(i)) + "\\b";
				glipllkeywordPatterns << str.c_str();
			}

			foreach(const QString& pattern, glipllkeywordPatterns)
			{
				rule.pattern = QRegExp(pattern);
				rule.format = &glipLayoutLoaderKeywordFormat;
				highlightingRules.append(rule);
			}

		// GLIP Uniform Loader Keywords : 
			QStringList glipulkeywordPatterns;

			for(int i=0; i<Glip::Modules::UL_NumKeywords; i++)
			{
				std::string str = std::string("\\b") + UniformsVarsLoader::getKeyword(static_cast<UniformVarsLoaderKeyword>(i)) + "\\b";
				glipulkeywordPatterns << str.c_str();
			}

			foreach(const QString& pattern, glipulkeywordPatterns)
			{
				rule.pattern = QRegExp(pattern);
				rule.format = &glipUniformLoaderKeywordFormat;
				highlightingRules.append(rule);
			}

		// Class : 
			/*classFormat.setFontWeight(QFont::Bold);
			classFormat.setForeground(Qt::darkMagenta);
			rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
			rule.format = classFormat;
			highlightingRules.append(rule);*/

		// Single line comment : 
			//singleLineCommentFormat.setForeground(QColor(51,153,255));
			rule.pattern = QRegExp("//[^\n]*");
			rule.format = &singleLineCommentFormat;
			highlightingRules.append(rule);

		// Mult line comment : 
			//multiLineCommentFormat.setForeground(QColor(51,153,255));
			commentStartExpression = QRegExp("/\\*");
			commentEndExpression = QRegExp("\\*/");
	
		// Quotation ; 
			/*quotationFormat.setForeground(QColor(51,255,51));
			rule.pattern = QRegExp("\".*\"");
			rule.format = quotationFormat;
			highlightingRules.append(rule);*/

		// function : 
			/*functionFormat.setFontItalic(true);
			functionFormat.setForeground(Qt::blue);
			rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
			rule.format = functionFormat;
			highlightingRules.append(rule);*/
	}

	void Highlighter::highlightBlock(const QString &text)
	{
		if( highlightEnabled )
		{
			foreach(const HighlightingRule &rule, highlightingRules)
			{
				QRegExp expression(rule.pattern);
				int index = expression.indexIn(text);
				while(index>=0)
				{
					int length = expression.matchedLength();
					setFormat(index, length, *rule.format);
					index = expression.indexIn(text, index + length);
				}
			}
			setCurrentBlockState(0);

			// Multi-line comment : 
			int startIndex = 0;
			if (previousBlockState() != 1)
				startIndex = commentStartExpression.indexIn(text);

			while(startIndex>=0)
			{
				int endIndex = commentEndExpression.indexIn(text, startIndex);
				int commentLength;
				if(endIndex==-1)
				{
					setCurrentBlockState(1);
					commentLength = text.length() - startIndex;
				}
				else
					commentLength = endIndex - startIndex + commentEndExpression.matchedLength();

				setFormat(startIndex, commentLength, multiLineCommentFormat);
				startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
			}
		}

		// Search expression :
		if(!searchExpression.isEmpty())
		{
			int index = searchExpression.indexIn(text);
			while(index>=0)
			{
				int length = searchExpression.matchedLength();
				setFormat(index, length, searchExpressionFormat);
				index = searchExpression.indexIn(text, index + length);
			}
		}
	}

	void Highlighter::updateSettings(const CodeEditorSettings& settings)
	{
		// Color : 
		glslkeywordFormat.setForeground(		settings.getGLSLKeywordColor() );
		glslfunctionFormat.setForeground(		settings.getGLSLFunctionColor() );
		glipLayoutLoaderKeywordFormat.setForeground(	settings.getGLIPLayoutLoaderKeywordColor() );
		glipUniformLoaderKeywordFormat.setForeground(	settings.getGLIPUniformLoaderKeywordColor() );
		singleLineCommentFormat.setForeground(		settings.getCommentsColor() );
		multiLineCommentFormat.setForeground(		settings.getCommentsColor() );
		searchExpressionFormat.setBackground(		settings.getSearchColor() );

		// Font : 
		glslkeywordFormat.setFont(			settings.getKeywordFont() );
		glslfunctionFormat.setFont(			settings.getKeywordFont() );	
		glipLayoutLoaderKeywordFormat.setFont(		settings.getKeywordFont() );
		glipUniformLoaderKeywordFormat.setFont(		settings.getKeywordFont() );
		singleLineCommentFormat.setFont(		settings.getKeywordFont() );
		multiLineCommentFormat.setFont(			settings.getKeywordFont() );

		// General highlighting (only if highlight was allowed for this editor) :
		highlightEnabled = settings.isHighlightEnabled();

		// Update : 
		rehighlight();
	}

	void Highlighter::setSearchHighlight(const QRegExp& expression)
	{
		searchExpression = expression;

		// Update : 
		rehighlight();
	}	

	void Highlighter::clearSearchHighlight(void)
	{
		searchExpression = QRegExp();

		// Update : 
		rehighlight();
	}

// LineNumberArea 
	LineNumberArea::LineNumberArea(CodeEditor *editor)
	 : QWidget(editor), codeEditor(editor)
	{ }

	void LineNumberArea::paintEvent(QPaintEvent *event)
	{
		codeEditor->lineNumberAreaPaintEvent(event);
	}

	QSize LineNumberArea::sizeHint() const
	{
		return QSize(codeEditor->lineNumberAreaWidth(), 0);
	}

// CodeEditor
	CodeEditor::CodeEditor(QWidget *parent)
	 : 	QPlainTextEdit(parent), 
		currentFilename(""),
		highlightLine(false),
		lineNumberArea(NULL),
		highLighter(NULL)
	{
		lineNumberArea = new LineNumberArea(this);
		updateLineNumberAreaWidth(0);

		highLighter = new Highlighter(document());

		QObject::connect(this, 			SIGNAL(blockCountChanged(int)), 	this, SLOT(updateLineNumberAreaWidth(int)));
		QObject::connect(this,			SIGNAL(updateRequest(QRect,int)), 	this, SLOT(updateLineNumberArea(QRect,int)));
		QObject::connect(this, 			SIGNAL(cursorPositionChanged()),	this, SLOT(highlightCurrentLine()));
		QObject::connect(document(), 		SIGNAL(modificationChanged(bool)), 	this, SIGNAL(modified(bool)));
	}

	CodeEditor::~CodeEditor(void)
	{
		blockSignals(true);

		delete highLighter;
		delete lineNumberArea;
	}

	int CodeEditor::lineNumberAreaWidth(void) const
	{
		int digits = std::floor( std::log10( qMax(1, blockCount()) ) ) + 1;

		//int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
		int space = 3 + fontMetrics().width( '0' ) * digits;

		return space;
	}

	void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
	{
		QPainter 	painter(lineNumberArea);
		QColor 		bg = palette().background().color().lighter(130),
				ed = palette().text().color().lighter(70);

		painter.fillRect(event->rect(), bg);

		QTextBlock 	block 		= firstVisibleBlock();
		int 		blockNumber 	= block.blockNumber();
		int 		top 		= (int) blockBoundingGeometry(block).translated(contentOffset()).top();
		int 		bottom 		= top + (int) blockBoundingRect(block).height();

		QFontMetrics metrics(font);

		while (block.isValid() && top <= event->rect().bottom())
		{
			if (block.isVisible() && bottom >= event->rect().top())
			{
				QString number = QString::number(blockNumber + 1);
				painter.setPen(ed);
				//painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
				painter.drawText(0, top, lineNumberArea->width(), metrics.height(), Qt::AlignRight | Qt::AlignVCenter, number);
			}

			block = block.next();
			top = bottom;
			bottom = top + (int) blockBoundingRect(block).height();
			++blockNumber;
		}
	}

	void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
	{
		this->setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
	}

	void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
	{
		if(lineNumberArea==NULL)
			return ;

		if(dy)
			lineNumberArea->scroll(0, dy);
		else
			lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

		if(rect.contains(viewport()->rect()))
			updateLineNumberAreaWidth(0);
	}
	
	void CodeEditor::resizeEvent(QResizeEvent *e)
	{
		QPlainTextEdit::resizeEvent(e);

		QRect cr = contentsRect();
		lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
	}

	void CodeEditor::keyPressEvent(QKeyEvent* e)
	{
		// Ensure multi-line tabulations : 
		if(e->key()==Qt::Key_Tab || e->key()==Qt::Key_Backtab)
		{
			// Grab all the current selection : 
			QString selectedText = textCursor().selectedText();

			// Replace unicode : 
			selectedText.replace(QString::fromWCharArray(L"\u2029"), "\n");

			QTextCursor cursor = textCursor();

			int	startSel = cursor.selectionStart(), 
				endSel = cursor.selectionEnd();

			// If no selection, process the tab / backtab as usual : 
			if(endSel-startSel<=0 && e->key()==Qt::Key_Tab)
			{
				QPlainTextEdit::keyPressEvent(e);
				return ;
			}
			else if(endSel-startSel<=0 && e->key()==Qt::Key_Backtab)
			{
				cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
				cursor.insertText( cursor.selectedText().replace("\t","") );
				return ;
			}

			cursor.setPosition(endSel, QTextCursor::KeepAnchor);
			QTextBlock endBlock = cursor.block();

			cursor.setPosition(startSel, QTextCursor::KeepAnchor);
			QTextBlock block = cursor.block();

			for(; block.isValid() && !(endBlock < block); block = block.next())
			{
				if (!block.isValid())
					continue;

				cursor.movePosition(QTextCursor::StartOfLine);
				cursor.clearSelection();

				if(e->key()==Qt::Key_Tab)
					cursor.insertText("\t");
				else
				{
					cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
					cursor.insertText( cursor.selectedText().replace("\t","") );
				}
				
				cursor.movePosition(QTextCursor::NextBlock);
			}
		}
		else // Otherwise, propagate : 
			QPlainTextEdit::keyPressEvent(e);
	}

	void CodeEditor::contextMenuEvent(QContextMenuEvent* event)
	{
		QMenu *menu = createStandardContextMenu();
		
		menu->addSeparator();

		for(int k=0; k<subMenus.count(); k++)
			menu->addMenu(subMenus[k]);

		menu->exec(event->globalPos());
		delete menu;
	}

	void CodeEditor::highlightCurrentLine(void)
	{
		if(highlightLine)
		{
			QList<QTextEdit::ExtraSelection> extraSelections;

			if (!isReadOnly())
			{
				QTextEdit::ExtraSelection selection;

				QColor lineColor = palette().background().color().lighter(130);			

				selection.format.setBackground(lineColor);
				selection.format.setProperty(QTextFormat::FullWidthSelection, true);
				selection.cursor = textCursor();
				selection.cursor.clearSelection();
				extraSelections.append(selection);
			}

			setExtraSelections(extraSelections);
		}
	}

	void CodeEditor::clearHighlightOfCurrentLine(void)
	{
		// Prevent clear, if it was not already in place.
		if(highlightLine)	
		{
			QList<QTextEdit::ExtraSelection> extraSelections;

			if(!isReadOnly())
			{
				QTextEdit::ExtraSelection selection;

				QColor lineColor = palette().background().color();			

				selection.format.setBackground(lineColor);
				selection.format.setProperty(QTextFormat::FullWidthSelection, true);
				selection.cursor = textCursor();
				selection.cursor.clearSelection();
				extraSelections.append(selection);
			}

			setExtraSelections(extraSelections);
		}
	}

	bool CodeEditor::empty(void) const
	{
		return document()->isEmpty();
	}

	const QString& CodeEditor::getFilename(void) const
	{
		return currentFilename;
	}

	void CodeEditor::setFilename(const QString& newFilename)
	{
		currentFilename = newFilename;
		emit titleChanged();
	}

	QString CodeEditor::getPath(void) const
	{
		if(currentFilename.isEmpty())
			return "";
		else
		{
			QFileInfo path(currentFilename);
			return path.path();
		}
	}

	QString CodeEditor::getRawTitle(void) const
	{
		QFileInfo path(currentFilename);

		if(path.exists())
			return path.fileName();
		else
			return "<Unnamed>";
	}

	QString CodeEditor::getTitle(void) const
	{
		QFileInfo path(currentFilename);
		QString fileName;
		
		if(path.exists())
			fileName = path.fileName();
		else
			fileName = "Unnamed.ppl";

		if( isModified() )
			fileName += " *";

		return fileName;
	}
	
	std::string CodeEditor::getCurrentContent(void) const
	{
		return toPlainText().toStdString();
	}

	bool CodeEditor::isModified(void) const
	{
		return document()->isModified();
	}

	bool CodeEditor::canBeSaved(void) const
	{
		QFileInfo path(currentFilename);
		return path.isWritable();
	}

	void CodeEditor::open(QString newFilename)
	{
		if(newFilename.isEmpty())
			newFilename = currentFilename;

		QFile file(newFilename);

		if(!file.open(QFile::ReadOnly | QFile::Text))
		{
			QMessageBox::warning(this, tr("Error : "), tr("Cannot read file %1 :\n%2.").arg(newFilename).arg(file.errorString()));
			return ;
		}

		QTextStream in(&file);

		// Prevent to understand a real modification :
		bool prevState = this->blockSignals(true);

		setPlainText(in.readAll());

		// Force the size of the margin :
		setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);

		this->blockSignals(prevState);

		// Set the filename : 
		setFilename(newFilename);
	}

	void CodeEditor::save(QString newFilename)
	{
		if(newFilename.isEmpty())
			newFilename = currentFilename;

		QFile file(newFilename);

		if(!file.open(QFile::WriteOnly | QFile::Text))
		{
			QMessageBox::warning(this, tr("Error : "),tr("Cannot write file %1 :\n%2.").arg(newFilename).arg(file.errorString()));
			return ;
		}

		// Write : 
		QTextStream out(&file);

		out << toPlainText();

		file.close();

		document()->setModified(false);

		// Set the filename : 
		setFilename(newFilename);
	}

	void CodeEditor::insert(const QString& text)
	{
		textCursor().insertText(text);
	}

	void CodeEditor::addSubMenu(QMenu* menu)
	{
		subMenus.append(menu);
	}

	void CodeEditor::updateSettings(const CodeEditorSettings& settings)
	{
		// Prevent the code from sending modification signal :
		blockSignals(true);
		document()->blockSignals(true);

		// Set the font : 
		setFont(settings.getEditorFont());
		document()->setDefaultFont(settings.getEditorFont());

		// Set the tabulation length :
		const int tabStop = settings.getNumberOfSpacesPerTabulation();
		QFontMetrics metrics(settings.getEditorFont());
		setTabStopWidth(tabStop * metrics.width(' '));

		// Set word wrap : 
		setWordWrapMode(settings.getWrapMode() );

		// Set line highlight :
		highlightLine = settings.isLineHighlightEnabled();

		if(!highlightLine)
			clearHighlightOfCurrentLine();

		// Propagate : 
		if(highLighter!=NULL)
			highLighter->updateSettings(settings);

		document()->blockSignals(false);
		blockSignals(false);
	}

	void CodeEditor::search(QRegExp expression, QTextDocument::FindFlags flags)
	{
		if(highLighter!=NULL)
			highLighter->setSearchHighlight(expression);

		QTextCursor cursor = document()->find(expression, textCursor(), flags);
		
		// Not found, cycle around : 
		if(cursor.isNull() && (flags & QTextDocument::FindBackward)>0)
			cursor = document()->find(expression, document()->characterCount()-1, flags);
		else if(cursor.isNull())
			cursor = document()->find(expression, 0, flags);

		// Move : 
		if(!cursor.isNull())
			setTextCursor(cursor);
	}

	void CodeEditor::replace(QRegExp expression, QTextDocument::FindFlags flags, QString text)
	{
		if(highLighter!=NULL)
			highLighter->setSearchHighlight(expression);

		QTextCursor currentCursor = textCursor();
		currentCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
		QTextCursor cursor = document()->find(expression, currentCursor, flags);

		// Not found, cycle around : 
		if(cursor.isNull() && (flags & QTextDocument::FindBackward)>0)
			cursor = document()->find(expression, document()->characterCount()-1, flags);
		else
			cursor = document()->find(expression, 0, flags);

		if(!cursor.isNull())
		{
			// Move : 
			setTextCursor(cursor);

			// Replace :
			cursor.removeSelectedText();
			cursor.insertText(text);
		}
	}

	void CodeEditor::replaceAll(QRegExp expression, QTextDocument::FindFlags flags, QString text)
	{
		// TODO
	}

	void CodeEditor::clearSearch(void)
	{
		if(highLighter!=NULL)
			highLighter->clearSearchHighlight();
	}

// SearchAndReplaceWidget :
	SearchAndReplaceWidget::SearchAndReplaceWidget(QWidget* parent)
	 : 	QWidgetAction(parent),
		widget(parent),
		layout(&widget),
		searchLabel("Search : ", &widget),
		replaceLabel("Replace : ", &widget),
		searchPattern(&widget),
		replaceString(&widget),
		worldOnlyCheck("World Only", &widget),
		matchCaseCheck("Match case", &widget),
		backwardSearchCheck("Backward search", &widget),
		findButton("Find", &widget),
		replaceButton("Replace", &widget),
		replaceAllButton("Replace All", &widget),
		clearButton("Clear", &widget)
	{
		layout.addWidget(&searchLabel, 		0, 0);
		layout.addWidget(&searchPattern,	0, 1);
		layout.addWidget(&replaceLabel,		1, 0);
		layout.addWidget(&replaceString,	1, 1);
		layout.addWidget(&worldOnlyCheck,	2, 0, 1, 2);
		layout.addWidget(&matchCaseCheck,	3, 0, 1, 2);
		layout.addWidget(&backwardSearchCheck,	4, 0, 1, 2);
		layout.addWidget(&findButton,		5, 0);
		layout.addWidget(&clearButton,		5, 1);
		layout.addWidget(&replaceButton,	6, 0);
		layout.addWidget(&replaceAllButton,	6, 1);
		layout.addWidget(&errorString,		7, 0, 1, 2);

		errorString.hide();

		QObject::connect(&findButton, 		SIGNAL(released()), this, SLOT(prepareExpression()));
		QObject::connect(&replaceButton, 	SIGNAL(released()), this, SLOT(prepareExpression()));
		QObject::connect(&replaceAllButton, 	SIGNAL(released()), this, SLOT(prepareExpression()));
		QObject::connect(&clearButton, 		SIGNAL(released()), this, SIGNAL(clearSearch()));

		setDefaultWidget(&widget);
	}
	
	SearchAndReplaceWidget::~SearchAndReplaceWidget(void)
	{
		releaseWidget(&widget);
	}

	void SearchAndReplaceWidget::prepareExpression(void)
	{
		QPushButton* 	button 		= reinterpret_cast<QPushButton*>(QObject::sender());
		QString 	expression 	= searchPattern.text(),
				replacement	= replaceString.text();

		if(!expression.isEmpty())
		{
			QRegExp regExpression(expression);
			QTextDocument::FindFlags flags = 0;

			if(matchCaseCheck.isChecked())
			{
				regExpression.setCaseSensitivity(Qt::CaseSensitive);
				flags = flags | QTextDocument::FindCaseSensitively;	// actually, might be useless, see QTextDocument::find with QRegExp argument.
			}			
			else
				regExpression.setCaseSensitivity(Qt::CaseInsensitive);
			
			if(worldOnlyCheck.isChecked())
				flags = flags | QTextDocument::FindWholeWords;

			if(backwardSearchCheck.isChecked())
				flags = flags | QTextDocument::FindBackward;

			if(regExpression.isValid())
			{
				// No error to report : 
				errorString.hide();

				if(button==&findButton)
				{
					// Send the search request : 
					emit search(regExpression, flags);
				}
				else if(button==&replaceButton)	
				{
					// Send the replace request : 
					emit replace(regExpression, flags, replacement);
				}
				else if(button==&replaceAllButton)
				{
					// Send the replace all request : 
					emit replaceAll(regExpression, flags, replacement);
				}
			}			
			else
			{
				errorString.setText(tr("Error : %1").arg(regExpression.errorString()));
				errorString.show();
			}
		}
	}

// CodeEditorSettings :
	CodeEditorSettings::CodeEditorSettings(QWidget* parent)
	 : 	QWidget(parent),
		layout(this),
		glslKeywordColorLabel("GLSL Keywords"),
		glslFunctionColorLabel("GLSL Functions"),
		glipLayoutLoaderKeywordColorLabel("GLIP Layout Loader Keywords"),
		glipUniformLoaderKeywordColorLabel("GLIP Uniforms Loader Keywords"),
		commentsColorLabel("Comments"),
		searchColorLabel("Search"),
		highlightKeywordsCheck("Highlight keywords"),
		highlightCurrentLineCheck("Highlight current line"),
		okButton("OK"),
		applyButton("Apply"),
		cancelButton("Cancel"),
		resetButton("Reset"),
		defaultFontSize( fontInfo().pointSize() )
	{
		resetSettings();
		
		// Create the layout for the GUI : 
			// Colors : 
				layoutColors.addWidget(&glslKeywordColorLabel, 			0, 0);
				layoutColors.addWidget(&glslFunctionColorLabel, 		1, 0);
				layoutColors.addWidget(&glipLayoutLoaderKeywordColorLabel, 	2, 0);
				layoutColors.addWidget(&glipUniformLoaderKeywordColorLabel, 	3, 0);
				layoutColors.addWidget(&commentsColorLabel, 			4, 0);
				layoutColors.addWidget(&searchColorLabel, 			5, 0);

				layoutColors.addWidget(&glslKeywordColorButton, 		0, 1);
				layoutColors.addWidget(&glslFunctionColorButton,		1, 1);
				layoutColors.addWidget(&glipLayoutLoaderKeywordColorButton, 	2, 1);
				layoutColors.addWidget(&glipUniformLoaderKeywordColorButton, 	3, 1);
				layoutColors.addWidget(&commentsColorButton, 			4, 1);
				layoutColors.addWidget(&searchColorButton,			5, 1);

				groupColors.setTitle("Highlight colors");
				groupColors.setLayout(&layoutColors);

				// Connect : 
				connect(&glslKeywordColorButton,		SIGNAL(released()),	this, SLOT(changeColor()));
				connect(&glslFunctionColorButton,		SIGNAL(released()),	this, SLOT(changeColor()));
				connect(&glipLayoutLoaderKeywordColorButton,	SIGNAL(released()),	this, SLOT(changeColor()));
				connect(&glipUniformLoaderKeywordColorButton,	SIGNAL(released()),	this, SLOT(changeColor()));
				connect(&commentsColorButton,			SIGNAL(released()),	this, SLOT(changeColor()));
				connect(&searchColorButton,			SIGNAL(released()),	this, SLOT(changeColor()));

			// Font : 
				layoutFonts.addWidget(&editorFontButton);
				layoutFonts.addWidget(&keywordFontButton);

				groupFonts.setTitle("Fonts");
				groupFonts.setLayout(&layoutFonts);

				// Connect :
				connect(&editorFontButton,			SIGNAL(released()),	this, SLOT(changeFont()));
				connect(&keywordFontButton,			SIGNAL(released()),	this, SLOT(changeFont()));

			// Misc : 
				wrapModesBox.addItem( "Word wrap", 	QVariant(QTextOption::WordWrap) );
				wrapModesBox.addItem( "No wrap", 	QVariant(QTextOption::NoWrap) );

				tabSpacesSpin.setRange(0, 128);
				tabSpacesSpin.setPrefix("Tabulations length : ");

				layoutMisc.addWidget(&highlightKeywordsCheck);
				layoutMisc.addWidget(&highlightCurrentLineCheck);
				layoutMisc.addWidget(&wrapModesBox);
				layoutMisc.addWidget(&tabSpacesSpin);
				
				groupMisc.setTitle("Miscellaneous");
				groupMisc.setLayout(&layoutMisc);

			// General : 
				layout.addWidget(&groupFonts, 	0, 0, 2, 2);
				layout.addWidget(&groupMisc, 	2, 0, 4, 2);
				layout.addWidget(&groupColors, 	0, 2, 5, 2);
				layout.addWidget(&resetButton, 	7, 0);
				layout.addWidget(&cancelButton, 7, 1);
				layout.addWidget(&applyButton, 	7, 2);
				layout.addWidget(&okButton, 	7, 3);

				connect(&applyButton,				SIGNAL(released()),	this, SLOT(softApply()));
				connect(&okButton,				SIGNAL(released()),	this, SLOT(quitDialog()));
				connect(&cancelButton,				SIGNAL(released()),	this, SLOT(quitDialog()));
				connect(&resetButton,				SIGNAL(released()),	this, SLOT(resetSettings())); 

		// Final update :
			updateGUI();
	}

	CodeEditorSettings::~CodeEditorSettings(void)
	{ }

	void CodeEditorSettings::updateGUI(void)
	{
		// From Values to GUI...

		// Colors : 
		glslKeywordColorButton.setStyleSheet(			tr("background:%1;").arg(glslKeywordColor.name()) );
		glslFunctionColorButton.setStyleSheet(			tr("background:%1;").arg(glslFunctionColor.name()) );
		glipLayoutLoaderKeywordColorButton.setStyleSheet(	tr("background:%1;").arg(glipLayoutLoaderKeywordColor.name()) );
		glipUniformLoaderKeywordColorButton.setStyleSheet(	tr("background:%1;").arg(glipUniformLoaderKeywordColor.name()) );
		commentsColorButton.setStyleSheet(			tr("background:%1;").arg(commentsColor.name()) );
		searchColorButton.setStyleSheet(			tr("background:%1;").arg(searchColor.name()) );

		// Fonts : 
		editorFontButton.setText(tr("Editor : %1 (%2)").arg(editorFont.family()).arg(editorFont.pointSize()));
		editorFontButton.setFont(editorFont);

		keywordFontButton.setText(tr("Keywords : %1 (%2)").arg(keywordFont.family()).arg(keywordFont.pointSize()));
		keywordFontButton.setFont(keywordFont);

		// Misc : 
		highlightKeywordsCheck.setChecked( enableHighlight );
		highlightCurrentLineCheck.setChecked( highlightCurrentLine );
		wrapModesBox.setCurrentIndex( wrapModesBox.findData(QVariant(wrapMode)) );
		tabSpacesSpin.setValue( tabNumberOfSpaces );
	}

	void CodeEditorSettings::updateValues(void)
	{
		// From GUI to Values...

		// Colors :
		glslKeywordColor		= glslKeywordColorButton.palette().color(QPalette::Window);
		glslFunctionColor		= glslFunctionColorButton.palette().color(QPalette::Window);
		glipLayoutLoaderKeywordColor	= glipLayoutLoaderKeywordColorButton.palette().color(QPalette::Window);
		glipUniformLoaderKeywordColor	= glipUniformLoaderKeywordColorButton.palette().color(QPalette::Window);
		commentsColor			= commentsColorButton.palette().color(QPalette::Window);
		searchColor			= searchColorButton.palette().color(QPalette::Window);

		// Fonts :
		editorFont			= editorFontButton.font();
		keywordFont			= keywordFontButton.font();

		// Misc : 
		enableHighlight			= highlightKeywordsCheck.isChecked();
		highlightCurrentLine		= highlightCurrentLineCheck.isChecked();
		wrapMode			= static_cast<QTextOption::WrapMode>( wrapModesBox.itemData( wrapModesBox.currentIndex() ).toUInt() );
		tabNumberOfSpaces 		= tabSpacesSpin.value();

		// Propagate : 
		emit settingsModified();
	}

	void CodeEditorSettings::changeColor(void)
	{
		// Get the pushbutton : 
		QPushButton* target = reinterpret_cast<QPushButton*>(QObject::sender());

		QString title;

		if(target==&glslKeywordColorButton)
			title = "GLSL Keywords Color";
		else if(target==&glslFunctionColorButton)
			title = "GLSL Functions Color";
		else if(target==&glipLayoutLoaderKeywordColorButton)
			title = "GLIP Layout Loader Keywords Color";
		else if(target==&glipUniformLoaderKeywordColorButton)
			title = "GLIP Uniforms Loader Keyword";
		else if(target==&commentsColorButton)
			title = "Comments Color";
		else if(target==&searchColorButton)
			title = "Search Color";
		else
			throw Exception("CodeEditorSettings::changeColor - Unknown color picker (internal error).", __FILE__, __LINE__);

		QColor result = QColorDialog::getColor(target->palette().color(QPalette::Window), this, title);

		// If the user pressed 'Ok' : 
		if(result.isValid())
			target->setStyleSheet( tr("background:%1;").arg(result.name()) );
	}

	void CodeEditorSettings::changeFont(void)
	{
		QPushButton* target = reinterpret_cast<QPushButton*>(QObject::sender());

		QString title;

		if(target==&editorFontButton)
			title = "Editor Font";
		else if(target==&keywordFontButton)
			title = "Keyword Font";
		else
			throw Exception("CodeEditorSettings::changeFont - Unknown color picker (internal error).", __FILE__, __LINE__);

		bool ok = false;
		QFont result = QFontDialog::getFont(&ok, target->font(), this, title);

		if(ok)
		{
			result.setFixedPitch(true);

			if(target==&editorFontButton)
				editorFontButton.setText(tr("Editor : %1 (%2)").arg(result.family()).arg(result.pointSize()));
			else if(target==&keywordFontButton)
				keywordFontButton.setText(tr("Keywords : %1 (%2)").arg(result.family()).arg(result.pointSize()));

			target->setFont(result);
		}
	}

	void CodeEditorSettings::softApply(void)
	{
		updateValues();
	}

	void CodeEditorSettings::quitDialog(void)
	{
		QPushButton* sender = reinterpret_cast<QPushButton*>(QObject::sender());

		hide();

		if(sender==&okButton)
			updateValues();
	}

	void CodeEditorSettings::resetSettings(void)
	{
		// Colors : 
		glslKeywordColor		= QColor(255,	128,	0);
		glslFunctionColor		= QColor(85,	255,	0);
		glipLayoutLoaderKeywordColor	= QColor(255, 	51, 	255);
		glipUniformLoaderKeywordColor	= QColor(51, 	255, 	255);
		commentsColor			= QColor(51,	153,	255);
		searchColor			= QColor(0,	192,	192);

		// Fonts : 
		QFontDatabase db;

		editorFont = db.font("Source Code Pro", "Regular", defaultFontSize);
		editorFont.setFixedPitch(true);

		keywordFont = db.font("Source Code Pro", "Bold", defaultFontSize);
		editorFont.setFixedPitch(true);

		// Set the tabulation length :
		tabNumberOfSpaces = 8;

		// Wrap mode :
		wrapMode = QTextOption::WordWrap;

		// Highlight : 
		enableHighlight 	= true;
		highlightCurrentLine	= true;

		updateGUI();
	}

	const QColor& 			CodeEditorSettings::getGLSLKeywordColor(void) const			{ return glslKeywordColor; }
	const QColor& 			CodeEditorSettings::getGLSLFunctionColor(void) const			{ return glslFunctionColor; }
	const QColor& 			CodeEditorSettings::getGLIPLayoutLoaderKeywordColor(void) const		{ return glipLayoutLoaderKeywordColor; }
	const QColor& 			CodeEditorSettings::getGLIPUniformLoaderKeywordColor(void) const	{ return glipUniformLoaderKeywordColor; }
	const QColor& 			CodeEditorSettings::getCommentsColor(void) const			{ return commentsColor; }
	const QColor& 			CodeEditorSettings::getSearchColor(void) const				{ return searchColor; }
	const QFont&			CodeEditorSettings::getEditorFont(void) const				{ return editorFont; }
	const QFont& 			CodeEditorSettings::getKeywordFont(void) const				{ return keywordFont; }
	const QTextOption::WrapMode& 	CodeEditorSettings::getWrapMode(void) const				{ return wrapMode; }
	const int& 			CodeEditorSettings::getNumberOfSpacesPerTabulation(void) const		{ return tabNumberOfSpaces; }
	const bool& 			CodeEditorSettings::isHighlightEnabled(void) const			{ return enableHighlight; }
	const bool&			CodeEditorSettings::isLineHighlightEnabled(void) const			{ return highlightCurrentLine; }

	std::string CodeEditorSettings::getSettingsString(void) const
	{
		std::string str;

		Glip::Modules::VanillaParserSpace::Element element;

		element.noName 		= true;
		element.noArgument	= false;
		element.noBody		= true;

		#define SAVE_COLOR( varName ) \
			element.strKeyword = STR( varName ); \
			element.arguments.push_back( varName .name().toStdString() ); \
			str += element.getCode() + "\n"; \
			element.arguments.clear();

		#define SAVE_NUMBER( varName ) \
			element.strKeyword = STR( varName ); \
			element.arguments.push_back( to_string(static_cast<int>( varName )) ); \
			str += element.getCode() + "\n"; \
			element.arguments.clear(); \

		#define SAVE_TEST( varName, test ) \
			element.strKeyword = STR( varName ); \
			element.arguments.push_back( to_string(static_cast<int>( varName test )) ); \
			str += element.getCode() + "\n"; \
			element.arguments.clear();	

		#define SAVE_FONT( varName ) \
			element.strKeyword = STR( varName ); \
			element.arguments.push_back( varName .family().toStdString() ); \
			element.arguments.push_back( varName .styleName().toStdString() ); \
			element.arguments.push_back( to_string( varName .pointSize()) ); \
			str += element.getCode() + "\n"; \
			element.arguments.clear();
	
			SAVE_COLOR( glslKeywordColor )
			SAVE_COLOR( glslFunctionColor )
			SAVE_COLOR( glipLayoutLoaderKeywordColor )
			SAVE_COLOR( glipUniformLoaderKeywordColor )
			SAVE_COLOR( commentsColor )

			SAVE_NUMBER( tabNumberOfSpaces )
			SAVE_NUMBER( enableHighlight )
			SAVE_NUMBER( highlightCurrentLine )

			SAVE_TEST( wrapMode, ==QTextOption::WordWrap )
			
			SAVE_FONT( editorFont )
			SAVE_FONT( keywordFont )

		#undef SAVE_COLOR
		#undef SAVE_NUMBER
		#undef SAVE_TEST
		#undef SAVE_FONT

		return str;
	}
	
	void CodeEditorSettings::setSettingsFromString(const std::string& str)
	{
		Glip::Modules::VanillaParserSpace::VanillaParser parser(str);

		#define READ_COLOR( varName ) \
			if(it->strKeyword==STR( varName )) \
			{ \
				if( it->arguments.size()==1 ) \
					varName .setNamedColor( it->arguments.front().c_str() ); \
				else \
					throw Glip::Exception("CodeEditorSettings::setSettingsFromString - Cannot read color " STR( varName ) ".", __FILE__, __LINE__); \
			}

		#define READ_NUMBER( varName ) \
			if(it->strKeyword==STR( varName )) \
			{ \
				if( it->arguments.size()==1 ) \
				{ \
					if(!from_string(it->arguments.front(), varName)) \
						throw Glip::Exception("CodeEditorSettings::setSettingsFromString - Cannot read " STR( varName ) " : " + it->arguments.front() + ".", __FILE__, __LINE__); \
				} \
				else \
					throw Glip::Exception("CodeEditorSettings::setSettingsFromString - Cannot read " STR( varName ) ".", __FILE__, __LINE__); \
			}

		#define READ_TEST( varName, valTrue, valFalse ) \
			if(it->strKeyword==STR( varName )) \
			{ \
				if( it->arguments.size()==1 ) \
				{ \
					bool t = false; \
					if(!from_string(it->arguments.front(), t)) \
						throw Glip::Exception("CodeEditorSettings::setSettingsFromString - Cannot read " STR( varName ) " : " + it->arguments.front() + ".", __FILE__, __LINE__); \
					else if(t) \
						varName = valTrue; \
					else \
						varName = valFalse; \
				} \
				else \
					throw Glip::Exception("CodeEditorSettings::setSettingsFromString - Cannot read " STR( varName ) ".", __FILE__, __LINE__); \
			}	

		#define READ_FONT( varName ) \
			if(it->strKeyword==STR( varName )) \
			{ \
				if( it->arguments.size()==3 ) \
				{ \
					QString familyName = it->arguments[0].c_str(), \
						styleName  = it->arguments[1].c_str(); \
					int	pointSize; \
					\
					if(!from_string(it->arguments[2], pointSize)) \
						throw Glip::Exception("CodeEditorSettings::setSettingsFromString - Cannot read pointSize for font " STR( varName ) " : " + it->arguments[2] + ".", __FILE__, __LINE__); \
					\
					QFontDatabase db; \
					varName = db.font( familyName, styleName, pointSize); \
				} \
				else \
					throw Glip::Exception("CodeEditorSettings::setSettingsFromString - Cannot read " STR( varName ) ".", __FILE__, __LINE__); \
			} 

		for(std::vector<Glip::Modules::VanillaParserSpace::Element>::iterator it=parser.elements.begin(); it!=parser.elements.end(); it++)
		{
			READ_COLOR( glslKeywordColor )
			READ_COLOR( glslFunctionColor )
			READ_COLOR( glipLayoutLoaderKeywordColor )
			READ_COLOR( glipUniformLoaderKeywordColor )
			READ_COLOR( commentsColor )

			READ_NUMBER( tabNumberOfSpaces )
			READ_NUMBER( enableHighlight )
			READ_NUMBER( highlightCurrentLine )

			READ_TEST( wrapMode, QTextOption::WordWrap, QTextOption::NoWrap)
			
			READ_FONT( editorFont )
			READ_FONT( keywordFont )
		}
	}

// TemplateMenu : 
	const char* TemplateMenu::templatesName[numTemplates]		= { 	"Add path",
										"Include file",
										"Required format", 
										"Required pipeline", 
										"Texture format",
										"Geometry + StandardQuad",
										"Geometry + Grid2D",
										"Geometry + Grid3D",
										"Geometry + Custom Model",
										"Shared source", 
										"Shader source + GLSL fragment",
										"Shader source + GLSL vertex",
										"Include shared source",
										"Include shader",
										"Filter layout", 
										"Pipeline layout", 
										"Main pipeline layout",
										"Main pipeline indirection",
										"Input ports", 
										"Output ports", 
										"Filter instance",
										"Pipeline instance", 
										"Connection",
										"Full Pipeline structure",
										"Full MainPipeline structure"
									};

	const char* TemplateMenu::templatesCode[numTemplates]		= {	"ADD_PATH( )\n",
										"INCLUDE_FILE( )\n",
										"REQUIRED_FORMAT: ( )\n",
										"REQUIRED_PIPELINE: ( )\n",
										"TEXTURE_FORMAT: ( , , GL_RGB, GL_UNSIGNED_BYTE)\n",
										"GEOMETRY: (STANDARD_QUAD)\n",
										"GEOMETRY: (GRID_2D, , )\n",
										"GEOMETRY: (GRID_3D, , , )\n",
										"GEOMETRY: (CUSTOM_MODEL, , )\n{\n\tVERTEX( x, y, z, u, v)\n\t\n\tELEMENT(a, b, c, d)\n}\n",
										"SHARED_SOURCE: \n{\n\t\n}\n",
										"SHADER_SOURCE: \n{\n\t#version 130\n\t\n\tuniform sampler2D ;\n\tout vec4 ;\n\t\n\tvoid main()\n\t{\n\t\tvec2 pos = gl_TexCoord[0].st;\n\t\tvec4 col = textureLod( , pos, 0);\n\t\n\t\t = col;\n\t}\n}\n",
										"SHADER_SOURCE: \n{\n\t#version 130\n\t\n\tvoid main()\n\t{\n\t\tgl_FrontColor = gl_Color;\n\n\t\tgl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n\t\tgl_Position = ftransform();\n\t}\n}\n",
										"INCLUDE_SHARED_SOURCE:\n",
										"SHADER_SOURCE: ()\n",
										"FILTER_LAYOUT: ( , )\n",
										"PIPELINE_LAYOUT: \n{\n\t\n}\n",
										"PIPELINE_MAIN: \n{\n\t\n}\n",
										"PIPELINE_MAIN: ()\n",
										"INPUT_PORTS( )\n",
										"OUTPUT_PORTS( )\n",
										"FILTER_INSTANCE: ( )\n",
										"PIPELINE_INSTANCE: ( )\n",
										"CONNECTION( , , , )\n",
										"PIPELINE_LAYOUT: \n{\n\tINPUT_PORTS( )\n\tOUTPUT_PORTS( )\n\n\tFILTER_INSTANCE: ( )\n\n\tCONNECTION( , , , )\n\tCONNECTION( , , , )\n}\n",
										"PIPELINE_MAIN: \n{\n\tINPUT_PORTS( )\n\tOUTPUT_PORTS( )\n\n\tFILTER_INSTANCE: ( )\n\n\tCONNECTION( , , , )\n\tCONNECTION( , , , )\n}\n"
									};

	const char* TemplateMenu::templatesCodeWithHelp[numTemplates]	= {	"ADD_PATH( /* path */ )\n",
										"INCLUDE_FILE( /* filename */ )\n",
										"REQUIRED_FORMAT: /* name to use */( /* required format name */ )\n",
										"REQUIRED_PIPELINE: /* name to use */ ( /* required pipeline name */ )\n",
										"TEXTURE_FORMAT: /* name */ ( /* width */, /* height */, GL_RGB, GL_UNSIGNED_BYTE)\n",
										"GEOMETRY: /* name */ (STANDARD_QUAD)\n",
										"GEOMETRY: /* name */ (GRID_2D, /* interger : width */, /* interger : height */)\n",
										"GEOMETRY: /* name */ (GRID_3D, /* interger : width */, /* interger : height */, /* interger : depth */)\n",
										"GEOMETRY: /* name */ (CUSTOM_MODEL, /* GL primitive (GL_POINTS, GL_LINES, GL_TRIANGLES, etc.) */, /* It has texcoord embedded? true or false */)\n{\n\tVERTEX( x, y, z, u, v) /* depending on the model format */\n\t\n\tELEMENT(a, b, c, d) /* depending on the model format */\n}\n",
										"SHADER_SOURCE: /* name */\n{\n\t/* Code */\n}\n",
										"SHADER_SOURCE: /* name */\n{\n\t#version 130\n\t\n\tuniform sampler2D /* input texture name */;\n\tout vec4 /* output texture name */;\n\t\n\t// uniform vec3 someVariableYouWantToModify = vec3(0.0, 0.0, 0.0);\n\t\n\tvoid main()\n\t{\n\t\t// The current fragment position : \n\t\tvec2 pos = gl_TexCoord[0].st;\n\t\t// Read the base level of the texture at the current position : \n\t\tvec4 col = textureLod(/* input texture name */, pos, 0);\n\t\n\t\t/* output texture name */ = col;\n\t}\n}\n",
										"SHADER_SOURCE: /* name */\n{\n\t#version 130\n\t\n\tvoid main()\n\t{\n\t\tgl_FrontColor = gl_Color;\n\n\t\tgl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n\t\tgl_Position = ftransform();\n\t}\n}\n",
										"INCLUDE_SHARED_SOURCE:/* shared source name */\n",
										"SHADER_SOURCE: /* name */ ( /* filename */ )\n",
										"FILTER_LAYOUT: /* name */ ( /* output texture format name */, /* shader name */)\n",
										"PIPELINE_LAYOUT: /* name */\n{\n\t/* structure code */\n}\n",
										"PIPELINE_MAIN: /* name */\n{\n\t/* structure code */\n}\n",
										"PIPELINE_MAIN: /* name */ ( /* other pipeline name */ )\n",
										"INPUT_PORTS( /* ports names list */ )\n",
										"OUTPUT_PORTS( /* ports names list */ )\n",
										"FILTER_INSTANCE: /* instance name */ ( /* filter layout name */ )\n",
										"PIPELINE_INSTANCE: /* instance name */ ( /* pipeline layout name */ )\n",
										"CONNECTION( /* source instance name or THIS (for input port) */, /* port name */, /* source instance name or THIS (for output port) */, /* port name */)\n",
										"PIPELINE_LAYOUT: /* name */\n{\n\tINPUT_PORTS( /* list of ports names */ )\n\tOUTPUT_PORTS( /* list of ports names */ )\n\n\tFILTER_INSTANCE: /* instance name */ ( /* layout name */ )\n\n\t// Note that in the case that your connections names are not ambiguous, you are allowed to not declare any CONNETION. Connections will be made automatically.\n\tCONNECTION( /* source instance name or THIS (for input ports) */, /* port name */, /* source instance name or THIS (for output ports) */, /* port name */ )\n\tCONNECTION( /* source instance name or THIS (for input port) */, /* port name */, /* source instance name or THIS (for output port) */, /* port name */ )\n}\n",
										"PIPELINE_MAIN: /* name */\n{\n\tINPUT_PORTS( /* list of ports names */ )\n\tOUTPUT_PORTS( /* list of ports names */ )\n\n\tFILTER_INSTANCE: /* instance name */ ( /* layout name */ )\n\n\t// Note that in the case that your connections names are not ambiguous, you are allowed to not declare any CONNETION. Connections will be made automatically.\n\tCONNECTION( /* source instance name or THIS (for input ports) */, /* port name */, /* source instance name or THIS (for output ports) */, /* port name */ )\n\tCONNECTION( /* source instance name or THIS (for input port) */, /* port name */, /* source instance name or THIS (for output port) */, /* port name */ )\n}\n"
									};

	TemplateMenu::TemplateMenu(QWidget* parent)
	 :	QMenu("Insert Template", parent), 
		signalMapper(this),  
		addComments("Option : insert comments", this)
	{
		addComments.setCheckable(true);
		addAction(&addComments);

		// Init : 
		for(int k=0; k<numTemplates; k++)
		{
			templatesActions[k] = new QAction( QString(templatesName[k]), this );
			addAction(templatesActions[k]);
			signalMapper.setMapping(templatesActions[k], k);

			connect(templatesActions[k], SIGNAL(triggered()), &signalMapper, SLOT(map()));
		}

		connect(&signalMapper, SIGNAL(mapped(int)), this, SLOT(insertTemplateCalled(int)));

		addComments.setChecked(true);
	}

	TemplateMenu::~TemplateMenu(void) 
	{
		for(int k=0; k<numTemplates; k++)
			delete templatesActions[k];
	}

	void TemplateMenu::insertTemplateCalled(int k)
	{
		CodeTemplates ct = static_cast<CodeTemplates>(k);

		if(addComments.isChecked())
			emit insertTemplate(QString(templatesCodeWithHelp[ct]));
		else
			emit insertTemplate(QString(templatesCode[ct]));
	}

// ElementsMenu
	ElementsMenu::ElementsMenu(QWidget* parent)
	 : QMenu("Elements", parent), timer(this)
	{
		updateMenu();

		connect(&timer, SIGNAL(timeout()), this, SIGNAL(updateElements()));

		timer.setInterval(15 * 1000);
		timer.start();
	}

	ElementsMenu::~ElementsMenu(void)
	{
		clear();
	}

	void ElementsMenu::insertCalled(void)
	{
		QAction* sender = reinterpret_cast<QAction*>( QObject::sender() );

		emit insertElement(sender->toolTip());
	}

	void ElementsMenu::updateMenu(void)
	{
		if(menus.empty())
		{
			clear();
			addAction("No elements")->setEnabled(false);
		}		
		else
		{
			QList<QAction*> actionsList = actions();
			for(int k=0; k<actionsList.count(); k++)
				removeAction( actionsList[k] );

			for(std::map<CodeEditor*, QMenu*>::iterator it=menus.begin(); it!=menus.end(); it++)
				addMenu(it->second);
		}
	}
	
	void ElementsMenu::scan(CodeEditor* editor, LayoutLoader::PipelineScriptElements& elements)
	{
		std::map<CodeEditor*, QMenu*>::iterator it = menus.find(editor);

		if(!editor->empty())
		{
			QMenu* menu = new QMenu(editor->getTitle(), parentWidget());

			menu->addAction(tr("Include %1").arg(editor->getRawTitle()), this, SLOT(insertCalled()))->setToolTip(tr("INCLUDE_FILE(%1)\n").arg(editor->getRawTitle()));

			if(!editor->getPath().isEmpty())
				menu->addAction(tr("Add path..."), this, SLOT(insertCalled()))->setToolTip(tr("ADD_PATH(%1)\n").arg(editor->getPath()));

			// Load : 
				#define MAKE_LIST( listName, name ) \
					if(!elements. listName .empty()) \
					{ \
						QMenu* tmp = menu->addMenu(tr( name " (%1)").arg(elements. listName .size())); \
						\
						for(int k=0; k<elements. listName .size(); k++) \
							tmp->addAction(elements. listName [k].c_str(), this, SLOT(insertCalled()))->setToolTip(elements. listName [k].c_str()); \
	 				}

				MAKE_LIST( requiredFormats, 	"Required Formats" );
				MAKE_LIST( requiredGeometries,	"Required Geometries" );
				MAKE_LIST( requiredPipelines,	"Required Pipelines" );
				MAKE_LIST( formats,		"Formats" );
				MAKE_LIST( shaderSources,	"Shader Sources" );
				MAKE_LIST( geometries,		"Geometries" );
				MAKE_LIST( filtersLayout,	"Filter Layouts");

				#undef MAKE_LIST

				// Pipelines : 
				if(!elements.pipelines.empty())
				{
					QMenu* tmp = menu->addMenu(tr("Pipelines (%1)").arg(elements.pipelines.size()));

					for(int k=0; k<elements.pipelines.size(); k++)
					{
						tmp->addAction(elements.pipelines[k].c_str(), this, SLOT(insertCalled()))->setToolTip(elements.pipelines[k].c_str());

						QMenu* i = tmp->addMenu(tr("Inputs of %1 (%2)").arg(elements.pipelines[k].c_str()).arg(elements.pipelineInputs[k].size()));

						for(int ki=0; ki<elements.pipelineInputs[k].size(); ki++)
							i->addAction(elements.pipelineInputs[k][ki].c_str(), this, SLOT(insertCalled()))->setToolTip(elements.pipelineInputs[k][ki].c_str());

						QMenu* o = tmp->addMenu(tr("Outputs of %1 (%2)").arg(elements.pipelines[k].c_str()).arg(elements.pipelineOutputs[k].size()));

						for(int ko=0; ko<elements.pipelineOutputs[k].size(); ko++)
							o->addAction(elements.pipelineOutputs[k][ko].c_str(), this, SLOT(insertCalled()))->setToolTip(elements.pipelineOutputs[k][ko].c_str());
					}
				}

				if(!elements.mainPipeline.empty())
				{
					QMenu* tmp = menu->addMenu("Main Pipeline");

					tmp->addAction(elements.mainPipeline.c_str(), this, SLOT(insertCalled()))->setToolTip(elements.mainPipeline.c_str());

					QMenu* i = tmp->addMenu(tr("Inputs of %1 (%2)").arg(elements.mainPipeline.c_str()).arg(elements.mainPipelineInputs.size()));

					for(int ki=0; ki<elements.mainPipelineInputs.size(); ki++)
						i->addAction(elements.mainPipelineInputs[ki].c_str(), this, SLOT(insertCalled()))->setToolTip(elements.mainPipelineInputs[ki].c_str());

					QMenu* o = tmp->addMenu(tr("Outputs of %1 (%2)").arg(elements.mainPipeline.c_str()).arg(elements.mainPipelineOutputs.size()));

					for(int ko=0; ko<elements.mainPipelineOutputs.size(); ko++)
						o->addAction(elements.mainPipelineOutputs[ko].c_str(), this, SLOT(insertCalled()))->setToolTip(elements.mainPipelineOutputs[ko].c_str());
				}

			// Test and update : 
			if(menu->actions().isEmpty())
			{
				// clear : 
				remove(editor);

				// clean : 
				delete menu;
			}
			else if(it!=menus.end())
			{
				removeAction(it->second->menuAction());
				delete it->second;

				it->second = menu;
			}
			else
				menus[editor] = menu;

			updateMenu();
		}
	}

	void ElementsMenu::remove(CodeEditor* editor)
	{
		std::map<CodeEditor*, QMenu*>::iterator it = menus.find(editor);

		if(it!=menus.end())
		{
			removeAction(it->second->menuAction());
			delete it->second;
			menus.erase(it);

			if(isEmpty())
				addAction("No elements")->setEnabled(false);
		}
	}

// MainWidget :
	/*MainWidget::MainWidget(void)
	 : 	layout(this), 
		menuBar(this), 
		widgets(this), 
		fileMenu("File", this), 
		//openSaveInterface("CodePannel", "File", "*.ppl *.glsl *.ext *.uvd *.txt"),
		templateMenu(this), 
		elementsMenu(this),
	  	newTabAction(tr("&New tab"), this),
		saveAllAction("Save all", this),
		refreshAction("Compile", this),
		closeTabAction(tr("&Close"), this),
		showPathWidgetAction("Paths",this),
		//pathWidget(this),
		showEditorSettingsAction("Editor Settings", this),
		closeAllAction("Close all", this),
		aboutAction("About", this)
	{
		// Add the actions : 
		newTabAction.setShortcuts(QKeySequence::New);
		//openSaveInterface.enableShortcuts(true);

		connect(&newTabAction, 			SIGNAL(triggered()), 			this, SLOT(newTab()));
		//connect(&openSaveInterface, 		SIGNAL(openFile(const QStringList&)), 	this, SLOT(open(const QStringList&)));
		//connect(&openSaveInterface, 		SIGNAL(saveFile(void)), 		this, SLOT(save(void)));
		//connect(&openSaveInterface, 		SIGNAL(saveFileAs(const QString&)), 	this, SLOT(saveAs(const QString&)));
		connect(&saveAllAction, 		SIGNAL(triggered()), 			this, SLOT(saveAll()));
		connect(&closeAllAction, 		SIGNAL(triggered()), 			this, SLOT(closeAll()));
		connect(&templateMenu, 			SIGNAL(insertTemplate()), 		this, SLOT(insertTemplate()));
		connect(&elementsMenu,			SIGNAL(updateElements()),		this, SLOT(updateElements()));
		connect(&elementsMenu,			SIGNAL(insertElement(const QString&)),	this, SLOT(insertElement(const QString&)));

		QKeySequence qs(Qt::CTRL + Qt::Key_R);
		refreshAction.setShortcut(qs);
		refreshAction.setText(refreshAction.text() + " (" + qs.toString() + ")");
		connect(&refreshAction, 		SIGNAL(triggered()), 			this, SLOT(refresh()));

		closeTabAction.setStatusTip(tr("Close"));
		closeTabAction.setShortcuts(QKeySequence::Close);
		connect(&closeTabAction, 		SIGNAL(triggered()), 			this, SLOT(closeTab()));

		showPathWidgetAction.setStatusTip(tr("Show paths"));
		connect(&showPathWidgetAction, 		SIGNAL(triggered()), 			this, SLOT(switchPathWidget()));

		connect(&showEditorSettingsAction,	SIGNAL(triggered()), 			this, SLOT(showEditorSettings()));
		connect(&aboutAction, 			SIGNAL(triggered()), 			this, SLOT(aboutMessage()));

		connect(&widgets, 			SIGNAL(currentChanged(int)), 		this, SLOT(tabChanged(int)));

		// Movable : 	
		widgets.setMovable(true);

		// Menus :
		fileMenu.addAction(&newTabAction);
		//openSaveInterface.addToMenu(fileMenu);
		fileMenu.addAction(&saveAllAction);
		fileMenu.addAction(&closeTabAction);
		fileMenu.addAction(&closeAllAction);
	
		menuBar.addMenu(&fileMenu);
		menuBar.addMenu(&templateMenu);
		menuBar.addMenu(&elementsMenu);
		menuBar.addAction(&refreshAction);
		menuBar.addAction(&showPathWidgetAction);
		menuBar.addAction(&showEditorSettingsAction);
		menuBar.addAction(&aboutAction);

		// Add the first tab : 
		newTab();

		// Hide paths : 
		//pathWidget.hide();

		layout.addWidget(&menuBar);	
		layout.addWidget(&widgets);
		layout.setMargin(0);
		layout.setSpacing(0);
	}

	MainWidget::~MainWidget(void)
	{
		editorSettings.close();
	}

	void MainWidget::newTab(void)
	{
		CodeEditor* newEditor = new CodeEditor(this);
		newEditor->addSubMenu(&templateMenu);
		newEditor->addSubMenu(&elementsMenu);
		newEditor->updateSettings(editorSettings);
		widgets.addTab(newEditor, newEditor->getTitle());
		widgets.setCurrentWidget( newEditor );
		//widgets.setCurrentTabTextColor( QColor("#BBBBBB") );

		//connect(newEditor, SIGNAL(titleChanged()), this, SLOT(updateTitle()));
	}

	void MainWidget::open(const QStringList& filenames)
	{
		for(int k=0; k<filenames.count(); k++)
			openFile( filenames[k] );
	}

	void MainWidget::save(void)
	{
		if(widgets.count() > 0)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());

			if(e->getFilename().isEmpty())
			{
				//QString filename = openSaveInterface.saveAsDialog();
				//
				//if(!filename.isEmpty())			
				//	e->setFilename(filename);
				//else
				//	return ;
			}

			e->save();

			//openSaveInterface.reportSuccessfulSave(e->filename());

			updateCurrentToolTip();
		}
	}

	void MainWidget::save(const QString& filename)
	{
		if(widgets.count() > 0)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());

			e->setFilename(filename);
			e->save();

			//openSaveInterface.reportSuccessfulSave(filename);

			updateCurrentToolTip();
		}
	}

	void MainWidget::saveAs(const QString& filename)
	{
		if(widgets.count() > 0)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());

			e->setFilename(filename);
			e->save();

			//openSaveInterface.reportSuccessfulSave(filename);
			//openSaveInterface.enableSave(true);

			updateCurrentToolTip();
		}
	}

	void MainWidget::saveAll(void)
	{
		CodeEditor* original = reinterpret_cast<CodeEditor*>(widgets.currentWidget());

		for(int k=0; k<widgets.count(); k++)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.widget(k));

			widgets.setCurrentWidget(e);
			save();
		}

		if(widgets.count() > 0)
			widgets.setCurrentWidget(original);
	}

	void MainWidget::refresh(void)
	{
		if(widgets.count()>0)
		{
			// Update colors : 
			//widgets.setTabsTextColor(QColor("#BBBBBB") );
				
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());
			//if(!e->empty())
				//widgets.setCurrentTabTextColor(Qt::white);

			// Send code : 
			//requirePipelineCreation(getCurrentCode() + "\n");
		}
	}

	void MainWidget::closeTab(void)
	{
		if(widgets.count() > 0)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());

			//if(e->canBeClosed())
			//{
				elementsMenu.remove(e);
				widgets.removeTab(widgets.indexOf(e));
				delete e;
			//}
		}
	}

	void MainWidget::closeAll(void)
	{
		for(int k=0; k<widgets.count(); k++)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.widget(k));

			//if(e->canBeClosed())
			//{
				widgets.removeTab(widgets.indexOf(e));
				delete e;
			//}
		}
	}

	void MainWidget::switchPathWidget(void)
	{
		//pathWidget.show();
	}

	void MainWidget::updateTitle(void)
	{
		CodeEditor* e = reinterpret_cast<CodeEditor*>( QObject::sender() );

		widgets.setTabText( widgets.indexOf(e), tr("   %1   ").arg(e->getTitle()));
	}

	void MainWidget::insertTemplate(void)
	{
		if(widgets.count() > 0)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());

			//e->insert( templateMenu.getTemplateCode() );
		}
	}

	void MainWidget::showEditorSettings(void)
	{
		editorSettings.show();
	}

	void MainWidget::aboutMessage(void)
	{
		int pointSize = font().pointSize();

		QMessageBox::about(this, "GlipStudio", tr("<center><p style=\"font-family: times, serif; font-size:%2pt; font-style:bold\">GlipStudio</p></center><p style=\"font-family: times, serif; font-size:%1pt; font-style:bold\">GlipStudio is the IDE for GlipLib (OpenGL Image Processing Library). Find more information, documentation and examples at : <a href='http://glip-lib.sourceforge.net/'>http://glip-lib.sourceforge.net/</a>.</p><center><p style=\"font-size:%3pt; font-style:italic\">Copyright &copy; 2013, Ronan Kerviche, MIT License</p></center><p style=\"font-size:%4pt;\"> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: <BR> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. <BR> THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p><table><tr><td><b><i>Binary build date</i></b> : </td><td>%5; %6</td></tr><tr><td><b><i>Hardware vendor : </i></b></td><td>%7</td></tr><tr><td><b><i>Renderer : </i></b></td><td>%8</td></tr><tr><td><b><i>OpenGL version : </i></b></td><td>%9</td></tr><tr><td><b><i>GLSL version : </i></b></td><td>%10</td></tr></table>").arg(pointSize).arg(pointSize+4).arg(pointSize+1).arg(pointSize-2).arg(__DATE__).arg(__TIME__).arg(HandleOpenGL::getVendorName().c_str()).arg(HandleOpenGL::getRendererName().c_str()).arg(HandleOpenGL::getVersion().c_str()).arg(HandleOpenGL::getGLSLVersion().c_str()));
	}

	void MainWidget::tabChanged(int c)
	{
		if(widgets.count() > 0)
		{
			//openSaveInterface.enableSave(true);
			saveAllAction.setEnabled(true);
		}
		else
		{
			//openSaveInterface.enableSave(false);
			saveAllAction.setEnabled(false);
		}	
	}

	void MainWidget::updateElements(void)
	{
		for(int k=0; k<widgets.count(); k++)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.widget(k));

			//if(e->isModifiedTrigger())
			//	updateElementsOfEditor(e);
		}
	}

	void MainWidget::insertElement(const QString& element)
	{
		if(widgets.count() > 0)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());

			e->insert( element );
		}
	}

	std::string MainWidget::getCurrentFilename(void)
	{
		if(widgets.count() > 0)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());

			return e->getFilename().toStdString();
		}
		else
			return "";
	}

	std::string MainWidget::getCurrentCode(void)
	{
		if(widgets.count() > 0)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());

			//return e->currentContent();
		}
		else
			return "";
	}

	void MainWidget::updateCurrentToolTip(void)
	{
		CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());

		QString toolTip = "<table>";
			toolTip += tr("<tr><td><i>Filename</i></td><td>:</td><td>%1</td></tr>").arg(e->getTitle());
			toolTip += tr("<tr><td><i>Path</i></td><td>:</td><td>%1</td></tr>").arg(e->getPath());
		toolTip += "</table>";

		widgets.setTabToolTip(widgets.currentIndex(), toolTip);
	}

	const std::vector<std::string>& MainWidget::getPaths(void)
	{
		// Make sure the list of path contains the one of the current pipeline :
		if(widgets.count() > 0)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());

			//pathWidget.addPath( e->path().toStdString() );
		}

		//return pathWidget.getPaths();
	}

	void MainWidget::preparePipelineLoading(LayoutLoader& loader, const LayoutLoader::PipelineScriptElements& infos)
	{
		// Add path : 
		loader.clearPaths();
		loader.addToPaths( getPaths() );
	}

	bool MainWidget::canBeClosed(void)
	{
		bool test = true;

		for(int k=0; k<widgets.count() && test; k++)
		{
			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.widget(k));

			widgets.setCurrentWidget(e);
			//test = test && e->canBeClosed();	
		}

		return test;
	}

	void MainWidget::openFile(const QString& filename)
	{
		QFileInfo info( filename );

		if(info.exists())
		{
			if(widgets.count() > 0)
			{
				CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());
				if(!e->empty())
					newTab();
			}
			else
				newTab();

			CodeEditor* e = reinterpret_cast<CodeEditor*>(widgets.currentWidget());
	
			//e->setFilename( filename );
			//if(!)
			//	closeTab();
			//else
			//{
			//	// Append the path : 
			//	//pathWidget.addPath( e->path().toStdString() );
			//
			//	// Report : 
			//	//openSaveInterface.reportSuccessfulLoad( filename );
			//	//openSaveInterface.enableSave(true);
			//	updateCurrentToolTip();
			//
			//	// Scan : 
			//	updateElementsOfEditor(e);
			//}

			e->open(filename);
		}
	}

	void MainWidget::closeEvent(QEvent* event)
	{
		//pathWidget.close();
		editorSettings.close();

		event->accept();
	}

	void MainWidget::close(void)
	{
		//pathWidget.close();
		editorSettings.close();

		QWidget::close();
	}

	void MainWidget::updateElementsOfEditor(CodeEditor* e)
	{
		try
		{
			//LayoutLoader::PipelineScriptElements elements = scan(e->currentContent() + "\n");

			//elementsMenu.scan(e, elements);
		}
		catch(Exception& err)
		{
			elementsMenu.remove(e);
		}
	}*/

// MainWidget :
	MainWidget::MainWidget(void)
	 : 	currentPath("."),
		layout(this),
		menuBarLeft(this),
		menuBarRight(this),
		//newTabButton(this),
		//searchButton(this),
		mainMenu("Menu", this),
		searchContainerMenu("Find", this),
		newAction("+", this),
		openAction("Open", this),	
		saveAction("Save", this),
		saveAsAction("Save as", this),
		saveAllAction("Save all", this),
		closeAllAction("Close all", this),
		settingsAction("Preferences", this),
		compileAction("Compile", this),
		templateMenu(this),
		elementsMenu(this),
		searchAndReplaceWidget(this)
	{
		// Build Menu : 
		mainMenu.addAction(&openAction);
		mainMenu.addAction(&saveAction);
		mainMenu.addAction(&saveAsAction);
		mainMenu.addAction(&saveAllAction);
		mainMenu.addAction(&closeAllAction);
		mainMenu.addSeparator();
		mainMenu.addMenu(&templateMenu);
		mainMenu.addMenu(&elementsMenu);
		mainMenu.addSeparator();
		mainMenu.addAction(&settingsAction);

		// Build search menu : 
		searchContainerMenu.addAction(&searchAndReplaceWidget);
		/*searchButton.setMenu(&searchContainerMenu);
		searchButton.setPopupMode(QToolButton::InstantPopup);*/

		// Tab Settings : 
		tabBar.setExpanding(false);
		tabBar.setMovable(true);
		tabBar.setTabsClosable(true);

		// Button settings :
		menuBarLeft.addMenu(&mainMenu);
		menuBarLeft.addAction(&compileAction);
		menuBarLeft.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		
		menuBarRight.addAction(&newAction);
		menuBarRight.addMenu(&searchContainerMenu);
		menuBarRight.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

		/*newTabButton.setMinimumSize(menuBar.height()-4, menuBar.height()-4);
		newTabButton.setMaximumSize(menuBar.height()-4, menuBar.height()-4);
		searchButton.setMinimumSize(menuBar.height()-4, menuBar.height()-4);
		searchButton.setMaximumSize(menuBar.height()-4, menuBar.height()-4);*/

		topBar.addWidget(&menuBarLeft);
		topBar.addWidget(&tabBar);
		topBar.addWidget(&menuBarRight);
		//topBar.addWidget(&newTabButton);
		//topBar.addWidget(&searchButton);

		topBar.setMargin(0);
		topBar.setSpacing(1);

		layout.addLayout(&topBar);
		layout.addLayout(&stack);
		layout.setMargin(0);
		layout.setSpacing(0);

		// Signals : 
		QObject::connect(&tabBar, 			SIGNAL(currentChanged(int)), 					this, 			 SLOT(changeToTab(int)));
		QObject::connect(&tabBar,			SIGNAL(tabCloseRequested(int)),					this, 			 SLOT(closeTab(int)));
		//QObject::connect(&newTabButton,			SIGNAL(released(void)), 					this, 			 SLOT(addTab(void)));
		QObject::connect(&newAction,			SIGNAL(triggered(void)), 					this, 			 SLOT(addTab(void)));
		QObject::connect(&openAction,			SIGNAL(triggered(void)), 					this, 			 SLOT(open()));
		QObject::connect(&saveAction,			SIGNAL(triggered(void)), 					this, 			 SLOT(save()));
		QObject::connect(&saveAsAction,			SIGNAL(triggered(void)), 					this, 			 SLOT(saveAs()));
		QObject::connect(&saveAllAction,		SIGNAL(triggered(void)), 					this, 			 SLOT(saveAll()));
		QObject::connect(&closeAllAction,		SIGNAL(triggered(void)), 					this, 			 SLOT(closeAll()));
		QObject::connect(&settingsAction,		SIGNAL(triggered(void)), 					&settings, 		 SLOT(show()));
		QObject::connect(&templateMenu,			SIGNAL(insertTemplate(QString)),				this,			 SLOT(insert(QString)));
		QObject::connect(&elementsMenu,			SIGNAL(insertElement(QString)),					this,			 SLOT(insert(QString)));
		QObject::connect(&elementsMenu,			SIGNAL(updateElements(void)),					this,			 SLOT(updateElements(void)));
		QObject::connect(&settings,			SIGNAL(settingsModified(void)),					this,			 SLOT(updateSettings(void)));
		QObject::connect(&compileAction,		SIGNAL(triggered(void)), 					this, 			 SLOT(transferSourceCompilation(void)));
		//QObject::connect(&searchContainerMenu,		SIGNAL(aboutToShow(void)),					&searchAndReplaceWidget, SLOT(aboutToShow(void))); // Would give the focus to the search bu breaks the position of the menu...
		QObject::connect(&searchAndReplaceWidget,	SIGNAL(search(QRegExp, QTextDocument::FindFlags)),		this,			 SLOT(transferSearchRequest(QRegExp, QTextDocument::FindFlags)));
		QObject::connect(&searchAndReplaceWidget,	SIGNAL(replace(QRegExp, QTextDocument::FindFlags, QString)),	this,			 SLOT(transferReplaceRequest(QRegExp, QTextDocument::FindFlags, QString)));
		QObject::connect(&searchAndReplaceWidget,	SIGNAL(replaceAll(QRegExp, QTextDocument::FindFlags, QString)),	this,			 SLOT(transferReplaceAllRequest(QRegExp, QTextDocument::FindFlags, QString)));
		QObject::connect(&searchAndReplaceWidget,	SIGNAL(clearSearch(void)),					this,			 SLOT(transferClearSearchRequest(void)));

		// Create a new tab by default : 
		addTab();
	}

	MainWidget::~MainWidget(void)
	{ 
		while(tabBar.count()>0)
			closeTab(0, true);
	}

	CodeEditor* MainWidget::getCurrentEditor(void)
	{
		int tabID = tabBar.currentIndex();

		if(tabID<0)
			return NULL;

		QMap<int, CodeEditor*>::iterator it = editors.find(tabBar.tabData(tabID).toUInt());

		if(it!=editors.end())
			return (*it);
		else
			return NULL;
	}

	int MainWidget::getTabIndex(CodeEditor* editor)
	{
		if(editor==NULL || tabBar.count()==0)
			return -1;
		else
		{
			int c = 0;

			for( ; c<tabBar.count(); c++)
			{
				int idx = tabBar.tabData(c).toUInt();

				QMap<int, CodeEditor*>::iterator it = editors.find(idx);

				if(it!=editors.end())
					if((*it)==editor)
						return c;
			}
		
			return -1;
		}
	}

	void MainWidget::setCurrentPath(QString path)
	{
		QFileInfo info(path);
		currentPath = info.path();
	}

	void MainWidget::save(CodeEditor* editor)
	{
		if(editor==NULL)
			editor = getCurrentEditor();

		if(editor==NULL)
			return ;

		if(editor->canBeSaved())
		{
			editor->save();
			setCurrentPath(editor->getFilename());
		}
		else 
			saveAs(editor);
	}

	void MainWidget::saveAs(CodeEditor* editor, QString filename)
	{
		if(editor==NULL)
			editor = getCurrentEditor();

		if(editor==NULL)
			return ;

		if(filename.isEmpty())
			filename = QFileDialog::getSaveFileName(NULL, tr("Save Source File as ..."), currentPath, "Source (*.ppl *.shr *.uvd *.txt)");

		if(filename.isEmpty())
			return ;
		else
		{
			editor->save(filename);
			setCurrentPath(filename);
		}
	}

	void MainWidget::closeEvent(QCloseEvent* event)
	{
		std::cout << "Close event!" << std::endl;
		event->accept();
	}

	LayoutLoader::PipelineScriptElements MainWidget::scanSource(const std::string& code) const
	{
		LayoutLoader layoutLoader;
		LayoutLoader::PipelineScriptElements  elements;

		try
		{
			elements = layoutLoader.listElements(code);
		}
		catch(Glip::Exception& e)
		{ } // Forget it.

		return elements;
	}

	void MainWidget::addTab(const QString& filename)
	{
		static unsigned int counter = 0; 
	
		// Create the widget : 
		CodeEditor* ptr = new CodeEditor;
		editors[counter] = ptr;
		ptr->updateSettings(settings);
		ptr->addSubMenu(&templateMenu);
		ptr->addSubMenu(&elementsMenu);

		QObject::connect(ptr, SIGNAL(titleChanged(void)),	this, SLOT(tabTitleChanged(void)));
		QObject::connect(ptr, SIGNAL(modified(bool)), 		this, SLOT(documentModified(bool)));

		// Create the tab :
		stack.addWidget(ptr);
		int c = tabBar.addTab("Unnamed.ppl");
		tabBar.setTabData(c, counter);
		tabBar.setCurrentIndex(c);	

		counter++;

		if(!filename.isEmpty())
		{
			ptr->open(filename);
			LayoutLoader::PipelineScriptElements elements = scanSource(ptr->getCurrentContent());
			elementsMenu.scan(ptr, elements);
		}
	}

	void MainWidget::tabTitleChanged(void)
	{
		CodeEditor* ptr = reinterpret_cast<CodeEditor*>(QObject::sender());

		int tabID = getTabIndex(ptr);

		if(ptr!=NULL && tabID>=0)
		{
			tabBar.setTabText(tabID, ptr->getTitle());

			QString toolTip = "<table>";
			toolTip += tr("<tr><td><i>Filename</i></td><td>:</td><td>%1</td></tr>").arg(ptr->getRawTitle());
			toolTip += tr("<tr><td><i>Path</i></td><td>:</td><td>%1</td></tr>").arg(ptr->getPath());
			toolTip += "</table>";

			tabBar.setTabToolTip(tabID, toolTip);
		}
	}

	void MainWidget::documentModified(bool changed)
	{
		CodeEditor* ptr = reinterpret_cast<CodeEditor*>(QObject::sender());

		int tabID = getTabIndex(ptr);

		if(ptr!=NULL && tabID>=0)
			tabBar.setTabText(tabID, ptr->getTitle());
	}

	void MainWidget::insert(QString str)
	{
		CodeEditor* editor = getCurrentEditor();

		if(editor!=NULL)
			editor->insert(str);
	}

	void MainWidget::updateElements(void)
	{
		for(QMap<int, CodeEditor*>::iterator it=editors.begin(); it!=editors.end(); it++)
		{
			LayoutLoader::PipelineScriptElements elements = scanSource((*it)->getCurrentContent());
			elementsMenu.scan(*it, elements);
		}
	}

	void MainWidget::changeToTab(int tabID)
	{
		int w = tabBar.tabData(tabID).toUInt();

		if(editors.find(w)!=editors.end())
			stack.setCurrentWidget(editors[w]);
	}

	void MainWidget::open(QStringList filenameList)
	{
		if(filenameList.empty())
			filenameList = QFileDialog::getOpenFileNames(NULL, "Open Source File(s)", currentPath, "Source Files (*.ppl *.shr *.uvd *.txt)");

		if(!filenameList.empty())
		{
			for(QStringList::iterator it=filenameList.begin(); it!=filenameList.end(); it++)
				addTab(*it);

			setCurrentPath(filenameList.front());
		}
	}

	void MainWidget::save(void)
	{
		save(getCurrentEditor());
	}

	void MainWidget::saveAs(const QString& filename)
	{
		saveAs(getCurrentEditor());
	}

	void MainWidget::saveAll(void)
	{
		for(QMap<int, CodeEditor*>::iterator it=editors.begin(); it!=editors.end(); it++)
			save(*it);
	}

	void MainWidget::updateSettings(void)
	{
		for(QMap<int, CodeEditor*>::iterator it=editors.begin(); it!=editors.end(); it++)
			(*it)->updateSettings(settings);
	}

	void MainWidget::transferSourceCompilation(void)
	{
		CodeEditor* editor = getCurrentEditor();

		if(editor!=NULL)
			emit compileSource(editor->getCurrentContent(), reinterpret_cast<void*>(editor));
	}

	void MainWidget::transferSearchRequest(QRegExp expression, QTextDocument::FindFlags flags)
	{
		CodeEditor* editor = getCurrentEditor();

		if(editor!=NULL)
			editor->search(expression, flags);
	}

	void MainWidget::transferReplaceRequest(QRegExp expression, QTextDocument::FindFlags flags, QString replacement)
	{
		CodeEditor* editor = getCurrentEditor();

		if(editor!=NULL)
			editor->replace(expression, flags, replacement);
	}

	void MainWidget::transferReplaceAllRequest(QRegExp expression, QTextDocument::FindFlags flags, QString replacement)
	{
		CodeEditor* editor = getCurrentEditor();

		if(editor!=NULL)
			editor->replaceAll(expression, flags, replacement);
	}

	void MainWidget::transferClearSearchRequest(void)
	{
		for(QMap<int, CodeEditor*>::iterator it=editors.begin(); it!=editors.end(); it++)
			(*it)->clearSearch();
	}

	void MainWidget::closeTab(int tabID, bool imperative)
	{
		int idx = tabBar.tabData(tabID).toUInt();

		QMap<int, CodeEditor*>::iterator it = editors.find(idx);

		// If it exists, clear : 
		if(it!=editors.end())
		{
			if((*it)->isModified())
			{
				changeToTab(tabID);

				QMessageBox::StandardButton 	returnedButton;
				QMessageBox::StandardButtons	buttons;

				if(imperative)
					buttons = QMessageBox::Save | QMessageBox::Discard;
				else
					buttons = QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel;

				if((*it)->getFilename().isEmpty())
					returnedButton = QMessageBox::warning(NULL, tr("Warning!"), tr("New file has been modified.\n Do you want to save your changes?"), buttons);
				else
					returnedButton = QMessageBox::warning(NULL, tr("Warning!"), tr("The file %1 has been modified.\n Do you want to save your changes?").arg((*it)->getFilename()), buttons);

				if(returnedButton==QMessageBox::Save)
					return save(*it);
				else if(returnedButton == QMessageBox::Cancel)
					return ; // exit
			}

			elementsMenu.remove(*it);
			tabBar.removeTab(tabID);
			stack.removeWidget(*it);
			delete (*it);
			editors.erase(it);
		}
	}

	void MainWidget::closeTab(void)
	{
		closeTab(tabBar.currentIndex());
	}
	
	void MainWidget::closeAll(void)
	{
		while(tabBar.count()>0)
			closeTab(0);
	}

