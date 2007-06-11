// qtractorClipForm.cpp
//
/****************************************************************************
   Copyright (C) 2005-2006, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qtractorClipForm.h"

#include "qtractorAbout.h"
#include "qtractorClip.h"
#include "qtractorClipCommand.h"

#include "qtractorMainForm.h"
#include "qtractorSession.h"
#include "qtractorOptions.h"

#include <QMessageBox>
#include <QLineEdit>


//----------------------------------------------------------------------------
// qtractorClipForm -- UI wrapper form.

// Constructor.
qtractorClipForm::qtractorClipForm (
	QWidget *pParent, Qt::WFlags wflags ) : QDialog(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Initialize dirty control state.
	m_pClip = NULL;
	m_iDirtyCount = 0;

	// Set proper spin-box time scales and display format...
	qtractorSpinBox::DisplayFormat displayFormat = qtractorSpinBox::Frames;

	qtractorMainForm *pMainForm = qtractorMainForm::getInstance();
	if (pMainForm) {
		// Default display-format from global options...
		qtractorOptions *pOptions = pMainForm->options();
		if (pOptions) {
			switch (pOptions->iTransportTime) {
			case 2:
				displayFormat = qtractorSpinBox::BBT;
				break;
			case 1:
				displayFormat = qtractorSpinBox::Time;
				break;
			case 0:
			default:
				displayFormat = qtractorSpinBox::Frames;
				break;
			}
		}
	}

	m_ui.ClipStartSpinBox->setDisplayFormat(displayFormat);
	m_ui.ClipOffsetSpinBox->setDisplayFormat(displayFormat);
	m_ui.ClipLengthSpinBox->setDisplayFormat(displayFormat);
	m_ui.FadeInLengthSpinBox->setDisplayFormat(displayFormat);
	m_ui.FadeOutLengthSpinBox->setDisplayFormat(displayFormat);

	switch (displayFormat) {
	case qtractorSpinBox::BBT:
		m_ui.BbtRadioButton->setChecked(true);
		break;
	case qtractorSpinBox::Time:
		m_ui.TimeRadioButton->setChecked(true);
		break;
	case qtractorSpinBox::Frames:
	default:
		m_ui.FramesRadioButton->setChecked(true);
		break;
	}

	// Try to restore old window positioning.
	adjustSize();

	// UI signal/slot connections...
	QObject::connect(m_ui.ClipNameLineEdit,
		SIGNAL(textChanged(const QString&)),
		SLOT(changed()));
	QObject::connect(m_ui.FramesRadioButton,
		SIGNAL(toggled(bool)),
		SLOT(formatChanged()));
	QObject::connect(m_ui.TimeRadioButton,
		SIGNAL(toggled(bool)),
		SLOT(formatChanged()));
	QObject::connect(m_ui.BbtRadioButton,
		SIGNAL(toggled(bool)),
		SLOT(formatChanged()));
	QObject::connect(m_ui.ClipStartSpinBox,
		SIGNAL(valueChanged(unsigned long)),
		SLOT(changed()));
	QObject::connect(m_ui.ClipOffsetSpinBox,
		SIGNAL(valueChanged(unsigned long)),
		SLOT(changed()));
	QObject::connect(m_ui.ClipLengthSpinBox,
		SIGNAL(valueChanged(unsigned long)),
		SLOT(changed()));
	QObject::connect(m_ui.FadeInLengthSpinBox,
		SIGNAL(valueChanged(unsigned long)),
		SLOT(changed()));
	QObject::connect(m_ui.FadeInTypeComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.FadeOutLengthSpinBox,
		SIGNAL(valueChanged(unsigned long)),
		SLOT(changed()));
	QObject::connect(m_ui.FadeOutTypeComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.OkPushButton,
		SIGNAL(clicked()),
		SLOT(accept()));
	QObject::connect(m_ui.CancelPushButton,
		SIGNAL(clicked()),
		SLOT(reject()));
}


// Destructor.
qtractorClipForm::~qtractorClipForm (void)
{
}


// Populate (setup) dialog controls from settings descriptors.
void qtractorClipForm::setClip ( qtractorClip *pClip )
{
	// Clip properties cloning...
	m_pClip = pClip;

	// FIXME: Somehow, for the spin-box take, do one 
	// has to make the dialog visible before hand?
	// problem is that it makes the widget modeless :(
//	QDialog::show();

	qtractorTrack *pTrack = NULL;
	qtractorSession *pSession = NULL;
	if (m_pClip)
		pTrack = m_pClip->track();
	if (pTrack)
		pSession = pTrack->session();
	if (pSession) {
		// Set from global time-scale instance...
		qtractorTimeScale *pTimeScale = pSession->timeScale();
		m_ui.ClipStartSpinBox->setTimeScale(pTimeScale);
		m_ui.ClipOffsetSpinBox->setTimeScale(pTimeScale);
		m_ui.ClipLengthSpinBox->setTimeScale(pTimeScale);
		m_ui.FadeInLengthSpinBox->setTimeScale(pTimeScale);
		m_ui.FadeOutLengthSpinBox->setTimeScale(pTimeScale);
		// Initialize dialog widgets...
		m_ui.ClipNameLineEdit->setText(m_pClip->clipName());
		// Parameters...
		m_ui.ClipStartSpinBox->setValue(m_pClip->clipStart());
		m_ui.ClipOffsetSpinBox->setValue(m_pClip->clipOffset());
		m_ui.ClipLengthSpinBox->setValue(m_pClip->clipLength());
		// Fade In/Out...
		m_ui.FadeInLengthSpinBox->setValue(m_pClip->fadeInLength());
		m_ui.FadeInTypeComboBox->setCurrentIndex(
			indexFromFadeType(m_pClip->fadeInType()));
		m_ui.FadeOutLengthSpinBox->setValue(m_pClip->fadeOutLength());
		m_ui.FadeOutTypeComboBox->setCurrentIndex(
			indexFromFadeType(m_pClip->fadeOutType()));
	}

	// Backup clean.
	m_iDirtyCount = 0;

	// Done.
	stabilizeForm();
}


// Retrieve the accepted clip, if the case arises.
qtractorClip *qtractorClipForm::clip (void) const
{
	return m_pClip;
}


// Accept settings (OK button slot).
void qtractorClipForm::accept (void)
{
	// Save options...
	qtractorMainForm *pMainForm = qtractorMainForm::getInstance();
	if (m_iDirtyCount > 0 && m_pClip && pMainForm) {
		// Make changes undoable...
		qtractorClipCommand *pClipCommand
			= new qtractorClipCommand(tr("edit clip"));
		pClipCommand->renameClip(m_pClip, m_ui.ClipNameLineEdit->text());
		// Parameters...
		unsigned long iClipStart  = m_ui.ClipStartSpinBox->value();
		unsigned long iClipOffset = m_ui.ClipOffsetSpinBox->value();
		unsigned long iClipLength = m_ui.ClipLengthSpinBox->value();
		if (iClipStart  != m_pClip->clipStart()  ||
			iClipOffset != m_pClip->clipOffset() ||
			iClipLength != m_pClip->clipLength()) {
			pClipCommand->resizeClip(m_pClip,
				iClipStart, iClipOffset, iClipLength);
		}
		// Fade in...
		unsigned long iFadeInLength  = m_ui.FadeInLengthSpinBox->value();
		qtractorClip::FadeType fadeInType = fadeTypeFromIndex(
			m_ui.FadeInTypeComboBox->currentIndex());
		if (iFadeInLength  != m_pClip->fadeInLength()
			|| fadeInType != m_pClip->fadeInType())
			pClipCommand->fadeInClip(m_pClip, iFadeInLength, fadeInType);
		// Fade out...
		unsigned long iFadeOutLength  = m_ui.FadeOutLengthSpinBox->value();
		qtractorClip::FadeType fadeOutType = fadeTypeFromIndex(
			m_ui.FadeOutTypeComboBox->currentIndex());
		if (iFadeOutLength  != m_pClip->fadeOutLength()
			|| fadeOutType != m_pClip->fadeOutType())
			pClipCommand->fadeOutClip(m_pClip, iFadeOutLength, fadeOutType);
		// Do it (but make it undoable)...
		pMainForm->commands()->exec(pClipCommand);
		// Reset dirty flag.
		m_iDirtyCount = 0;
	}

	// Just go with dialog acceptance.
	QDialog::accept();
}


// Reject settings (Cancel button slot).
void qtractorClipForm::reject (void)
{
	bool bReject = true;

	// Check if there's any pending changes...
	if (m_iDirtyCount > 0) {
		switch (QMessageBox::warning(this,
			tr("Warning") + " - " QTRACTOR_TITLE,
			tr("Some settings have been changed.\n\n"
			"Do you want to apply the changes?"),
			tr("Apply"), tr("Discard"), tr("Cancel"))) {
		case 0:     // Apply...
			accept();
			return;
		case 1:     // Discard
			break;
		default:    // Cancel.
			bReject = false;
		}
	}

	if (bReject)
		QDialog::reject();
}


// Dirty up settings.
void qtractorClipForm::changed (void)
{
	m_iDirtyCount++;
	stabilizeForm();
}


// Display format has changed.
void qtractorClipForm::formatChanged (void)
{
	qtractorSpinBox::DisplayFormat displayFormat = qtractorSpinBox::Frames;

	if (m_ui.TimeRadioButton->isChecked())
		displayFormat = qtractorSpinBox::Time;
	else
	if (m_ui.BbtRadioButton->isChecked())
		displayFormat= qtractorSpinBox::BBT;

	m_ui.ClipStartSpinBox->setDisplayFormat(displayFormat);
	m_ui.ClipOffsetSpinBox->setDisplayFormat(displayFormat);
	m_ui.ClipLengthSpinBox->setDisplayFormat(displayFormat);

	m_ui.FadeInLengthSpinBox->setDisplayFormat(displayFormat);
	m_ui.FadeOutLengthSpinBox->setDisplayFormat(displayFormat);

	stabilizeForm();
}


// Stabilize current form state.
void qtractorClipForm::stabilizeForm (void)
{
	unsigned long iClipLength = m_ui.ClipLengthSpinBox->value();
	m_ui.FadeInTypeComboBox->setEnabled(
		m_ui.FadeInLengthSpinBox->value() > 0);
	m_ui.FadeInLengthSpinBox->setMaximum(iClipLength);
	m_ui.FadeOutTypeComboBox->setEnabled(
		m_ui.FadeOutLengthSpinBox->value() > 0);
	m_ui.FadeOutLengthSpinBox->setMaximum(iClipLength);

	bool bValid = (m_iDirtyCount > 0);
	bValid = bValid && !m_ui.ClipNameLineEdit->text().isEmpty();
	bValid = bValid && (iClipLength > 0);
	m_ui.OkPushButton->setEnabled(bValid);
}


// Fade type index converters.
qtractorClip::FadeType qtractorClipForm::fadeTypeFromIndex ( int iIndex ) const
{
	qtractorClip::FadeType fadeType = qtractorClip::Linear;

	switch (iIndex) {
	case 1:
		fadeType = qtractorClip::Quadratic;
		break;
	case 2:
		fadeType = qtractorClip::Cubic;
		break;
	default:
		break;
	}

	return fadeType;
}

int qtractorClipForm::indexFromFadeType ( qtractorClip::FadeType fadeType ) const
{
	int iIndex = 0;	// qtractorClip::Linear

	switch (fadeType) {
	case qtractorClip::Quadratic:
		iIndex = 1;
		break;
	case qtractorClip::Cubic:
		iIndex = 2;
		break;
	default:
		break;
	}

	return iIndex;
}


// end of qtractorClipForm.cpp
