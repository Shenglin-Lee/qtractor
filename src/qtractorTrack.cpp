// qtractorTrack.cpp
//
/****************************************************************************
   Copyright (C) 2005, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#include "qtractorAbout.h"
#include "qtractorTrack.h"

#include "qtractorAudioClip.h"
#include "qtractorMidiClip.h"

#include "qtractorSessionDocument.h"
#include "qtractorAudioEngine.h"
#include "qtractorMidiEngine.h"
#include "qtractorInstrument.h"

#include <qpainter.h>


//-------------------------------------------------------------------------
// qtractorTrack -- Track container.

// Constructor.
qtractorTrack::qtractorTrack ( qtractorSession *pSession, TrackType trackType )
	: m_colorBackground(Qt::yellow), m_colorForeground(Qt::darkBlue)
{
	m_pSession  = pSession;
	m_trackType = trackType;
	m_pBus      = NULL;

	m_iMidiTag     = 0;
	m_iMidiChannel = 0;

	m_clips.setAutoDelete(true);

	clear();

	// FIXME: Initial track (normalized) height?
	m_iHeight = 48;
}

// Default constructor.
qtractorTrack::~qtractorTrack (void)
{
	m_clips.clear();
}


// Reset track.
void qtractorTrack::clear (void)
{
	m_clips.clear();

	m_iMidiBankSelMethod = -1;
	m_iMidiBank          = -1;
	m_iMidiProgram       = -1;

	m_bRecord = false;
	m_bMute   = false;
	m_bSolo   = false;
}


// Track open method.
bool qtractorTrack::open (void)
{
	close();

	if (m_pSession == NULL)
		return false;

	// Depending on track type...
	qtractorEngine *pEngine = NULL;
	switch (m_trackType) {
		case qtractorTrack::Audio:
			pEngine = m_pSession->audioEngine();
			break;
		case qtractorTrack::Midi:
			pEngine = m_pSession->midiEngine();
			break;
		default:
			break;
	}

	// Got it?
	if (pEngine == NULL)
		return false;

	// (Re)assign the bus to the track.
	m_pBus = pEngine->findBus(m_sBusName);
	// Fallback to first usable one...
	if (m_pBus == NULL) {
		m_pBus = pEngine->busses().first();
		if (m_pBus)
			m_sBusName = m_pBus->busName();
	}

	// Done.
	return (m_pBus != NULL);
}


// Track close method.
void qtractorTrack::close (void)
{
	m_pBus = NULL;
}


// Session accessor.
qtractorSession *qtractorTrack::session (void) const
{
	return m_pSession;
}


// Track name accessors.
const QString& qtractorTrack::trackName (void) const
{
	return m_sTrackName;
}

void qtractorTrack::setTrackName ( const QString& sTrackName )
{
	m_sTrackName = sTrackName;
}


// Track type accessors.
qtractorTrack::TrackType qtractorTrack::trackType (void) const
{
	return m_trackType;
}

void qtractorTrack::setTrackType ( qtractorTrack::TrackType trackType )
{
	if (m_trackType == trackType)
		return;

	if (m_trackType == qtractorTrack::Midi)
		m_pSession->releaseMidiTag(this);

	m_trackType = trackType;

	if (m_trackType == qtractorTrack::Midi)
		m_pSession->acquireMidiTag(this);
}


// Record status accessors.
bool qtractorTrack::isRecord (void) const
{
	return m_bRecord;
}

void qtractorTrack::setRecord ( bool bRecord )
{
	m_bRecord = bRecord;
}


// Mute status accessors.
bool qtractorTrack::isMute (void) const
{
	return m_bMute;
}

void qtractorTrack::setMute ( bool bMute )
{
	m_bMute = bMute;

	if (m_pSession->isPlaying())
		m_pSession->trackMute(this, bMute);
}


// Solo status accessors.
bool qtractorTrack::isSolo (void) const
{
	return m_bSolo;
}

void qtractorTrack::setSolo ( bool bSolo )
{
	m_bSolo = bSolo;

	if (m_pSession->isPlaying())
		m_pSession->trackSolo(this, bSolo);

	m_pSession->setSoloTracks(bSolo);
}


// MIDI specific: track-tag accessors.
void qtractorTrack::setMidiTag ( unsigned short iMidiTag )
{
	m_iMidiTag = iMidiTag;
}

unsigned short qtractorTrack::midiTag (void) const
{
	return m_iMidiTag;
}


// MIDI specific: channel acessors.
void qtractorTrack::setMidiChannel ( unsigned short iMidiChannel )
{
	m_iMidiChannel = iMidiChannel;
}

unsigned short qtractorTrack::midiChannel (void) const
{
	return m_iMidiChannel;
}


// MIDI specific: bank accessors.
void qtractorTrack::setMidiBankSelMethod ( int iMidiBankSelMethod )
{
	m_iMidiBankSelMethod = iMidiBankSelMethod;
}

int qtractorTrack::midiBankSelMethod (void) const
{
	return m_iMidiBankSelMethod;
}


// MIDI specific: bank accessors.
void qtractorTrack::setMidiBank ( int iMidiBank )
{
	m_iMidiBank = iMidiBank;
}

int qtractorTrack::midiBank (void) const
{
	return m_iMidiBank;
}


// MIDI specific: program accessors.
void qtractorTrack::setMidiProgram ( int iMidiProgram )
{
	m_iMidiProgram = iMidiProgram;
}

int qtractorTrack::midiProgram (void) const
{
	return m_iMidiProgram;
}


// Assigned bus name accessors.
void qtractorTrack::setBusName ( const QString& sBusName )
{
	m_sBusName = sBusName;
}

const QString& qtractorTrack::busName (void) const
{
	return m_sBusName;
}


// Assigned audio bus accessors.
qtractorBus *qtractorTrack::bus (void) const
{
	return m_pBus;
}


// Normalized view height accessors.
unsigned short qtractorTrack::height (void) const
{
	return m_iHeight;
}

void qtractorTrack::setHeight ( unsigned short iHeight )
{
	m_iHeight = iHeight;
}


// Clip list management methods.
qtractorList<qtractorClip>& qtractorTrack::clips (void)
{
	return m_clips;
}


// Insert a new clip in garanteed sorted fashion.
void qtractorTrack::addClip ( qtractorClip *pClip )
{
	qtractorClip *pNextClip = m_clips.first();

	while (pNextClip && pNextClip->clipStart() < pClip->clipStart())
		pNextClip = pNextClip->next();

	if (pNextClip)
		m_clips.insertBefore(pClip, pNextClip);
	else
		m_clips.append(pClip);

	pClip->setTrack(this);
}


void qtractorTrack::unlinkClip ( qtractorClip *pClip )
{
	pClip->setTrack(NULL);
	m_clips.unlink(pClip);
}

void qtractorTrack::removeClip ( qtractorClip *pClip )
{
	pClip->setTrack(NULL);
	m_clips.remove(pClip);
}


// Background color accessors.
void qtractorTrack::setBackground ( const QColor& bg )
{
	m_colorBackground = bg;
}

const QColor& qtractorTrack::background (void) const
{
	return m_colorBackground;
}


// Foreground color accessors.
void qtractorTrack::setForeground ( const QColor& fg )
{
	m_colorForeground = fg;
}

const QColor& qtractorTrack::foreground (void) const
{
	return m_colorForeground;
}


// Generate a default track color.
QColor qtractorTrack::trackColor ( int iTrack )
{
	int c[3] = { 0xff, 0xcc, 0x99 };

	return QColor(c[iTrack % 3], c[(iTrack / 3) % 3], c[(iTrack / 9) % 3]);
}


// Track special process cycle executive.
void qtractorTrack::process ( qtractorClip *pClip,
	unsigned long iFrameStart, unsigned long iFrameEnd )
{
	// Emulate soloing/muting with zero gain;
	float fGain = 1.0;
	if ((m_pSession->soloTracks() && !m_bSolo) || m_bMute)
		fGain = 0.0;

	// Now, for every clip...
	while (pClip && pClip->clipStart() < iFrameEnd) {
		pClip->process(fGain, iFrameStart, iFrameEnd);
		pClip = pClip->next();
	}
}


// Track paint method.
void qtractorTrack::drawTrack ( QPainter *pPainter, const QRect& trackRect,
	unsigned long iTrackStart, unsigned long iTrackEnd, qtractorClip *pClip )
{
	int y = trackRect.y();
	int h = trackRect.height();

	if (pClip == NULL)
		pClip = m_clips.first();
		
	while (pClip) {
		unsigned long iClipStart = pClip->clipStart();
		if (iClipStart > iTrackEnd)
			break;
		unsigned long iClipEnd = iClipStart + pClip->clipLength();
		if (iClipStart < iTrackEnd && iClipEnd > iTrackStart) {
			unsigned long iClipOffset = 0;
			int x = trackRect.x();
			int w = trackRect.width();
			if (iClipStart >= iTrackStart) {
				x += session()->pixelFromFrame(iClipStart - iTrackStart);
			} else {
				iClipOffset = iTrackStart - iClipStart;
				x--;	// Give some clip left-border room.
			}
			if (iClipEnd < iTrackEnd) {
				w -= session()->pixelFromFrame(iTrackEnd - iClipEnd) + 1;
			} else {
				w++;	// Give some clip right-border room.
			}
			const QRect clipRect(x, y, w - x, h);
			pClip->drawClip(pPainter, clipRect, iClipOffset);
			if (pClip->isClipSelected()) {
				Qt::RasterOp rop = pPainter->rasterOp();
				pPainter->setRasterOp(Qt::NotROP);
				pPainter->fillRect(clipRect, Qt::gray);
				pPainter->setRasterOp(rop);
			}
		}
		pClip = pClip->next();
	}
}


// MIDI track instrument patching.
void qtractorTrack::setMidiPatch ( qtractorInstrumentList *pInstruments )
{
	if (m_iMidiProgram < 0)
	    return;

	qtractorMidiBus *pMidiBus
		= static_cast<qtractorMidiBus *> (m_pBus);
	if (pMidiBus == NULL)
	    return;

	int iBankSelMethod = m_iMidiBankSelMethod;
	const QString& sInstrumentName = pMidiBus->instrumentName(m_iMidiChannel);
	if (!sInstrumentName.isEmpty() && iBankSelMethod < 0)
		iBankSelMethod = (*pInstruments)[sInstrumentName].bankSelMethod();

	pMidiBus->setPatch(m_iMidiChannel, sInstrumentName,
		m_iMidiBank, m_iMidiProgram, iBankSelMethod);
}


// Document element methods.
bool qtractorTrack::loadElement ( qtractorSessionDocument *pDocument,
	QDomElement *pElement )
{
	qtractorTrack::setTrackName(pElement->attribute("name"));
	qtractorTrack::setTrackType(
		pDocument->loadTrackType(pElement->attribute("type")));

	// Load track children...
	for (QDomNode nChild = pElement->firstChild();
			!nChild.isNull();
				nChild = nChild.nextSibling()) {

		// Convert node to element...
		QDomElement eChild = nChild.toElement();
		if (eChild.isNull())
			continue;

		// Load (other) track properties..
		if (eChild.tagName() == "properties") {
			for (QDomNode nProp = eChild.firstChild();
					!nProp.isNull();
						nProp = nProp.nextSibling()) {
				// Convert property node to element...
				QDomElement eProp = nProp.toElement();
				if (eProp.isNull())
					continue;
				if (eProp.tagName() == "bus")
					qtractorTrack::setBusName(eProp.text());
				else if (eProp.tagName() == "midi-channel")
					qtractorTrack::setMidiChannel(eProp.text().toUShort());
				else if (eProp.tagName() == "midi-bank-sel-method")
					qtractorTrack::setMidiBankSelMethod(eProp.text().toInt());
				else if (eProp.tagName() == "midi-bank")
					qtractorTrack::setMidiBank(eProp.text().toInt());
				else if (eProp.tagName() == "midi-program")
					qtractorTrack::setMidiProgram(eProp.text().toInt());
			}
		}
		else
		// Load track state..
		if (eChild.tagName() == "state") {
			for (QDomNode nStat = eChild.firstChild();
					!nStat.isNull();
						nStat = nStat.nextSibling()) {
				// Convert state node to element...
				QDomElement eStat = nStat.toElement();
				if (eStat.isNull())
					continue;
				if (eStat.tagName() == "mute")
					qtractorTrack::setMute(pDocument->boolFromText(eStat.text()));
				else if (eStat.tagName() == "solo")
					qtractorTrack::setSolo(pDocument->boolFromText(eStat.text()));
				else if (eStat.tagName() == "record")
					qtractorTrack::setRecord(pDocument->boolFromText(eStat.text()));
			}
		}
		else
		if (eChild.tagName() == "view") {
			for (QDomNode nView = eChild.firstChild();
					!nView.isNull();
						nView = nView.nextSibling()) {
				// Convert view node to element...
				QDomElement eView = nView.toElement();
				if (eView.isNull())
					continue;
				if (eView.tagName() == "height") {
					qtractorTrack::setHeight(eView.text().toUShort());
				} else if (eView.tagName() == "background-color") {
					QColor bg; bg.setNamedColor(eView.text());
					qtractorTrack::setBackground(bg);
				} else if (eView.tagName() == "foreground-color") {
					QColor fg; fg.setNamedColor(eView.text());
					qtractorTrack::setForeground(fg);
				}
			}
		}
		else
		// Load clips...
		if (eChild.tagName() == "clips") {
			for (QDomNode nClip = eChild.firstChild();
					!nClip.isNull();
						nClip = nClip.nextSibling()) {
				// Convert clip node to element...
				QDomElement eClip = nClip.toElement();
				if (eClip.isNull())
					continue;
				if (eClip.tagName() == "clip") {
					qtractorClip *pClip = NULL;
					switch (trackType()) {
						case qtractorTrack::Audio:
							pClip = new qtractorAudioClip(this);
							break;
						case qtractorTrack::Midi:
							pClip = new qtractorMidiClip(this);
							break;
						case qtractorTrack::None:
						default:
							break;
					}
					if (pClip == NULL)
						return false;
					if (!pClip->loadElement(pDocument, &eClip))
						return false;
					qtractorTrack::addClip(pClip);
				}
			}
		}
	}

	return true;
}


bool qtractorTrack::saveElement ( qtractorSessionDocument *pDocument,
	QDomElement *pElement )
{
	pElement->setAttribute("name", qtractorTrack::trackName());
	pElement->setAttribute("type",
		pDocument->saveTrackType(qtractorTrack::trackType()));

	// Save track properties...
	QDomElement eProps = pDocument->document()->createElement("properties");
	pDocument->saveTextElement("bus", qtractorTrack::busName(), &eProps);
	if (qtractorTrack::trackType() == qtractorTrack::Midi) {
		pDocument->saveTextElement("midi-channel",
			QString::number(qtractorTrack::midiChannel()), &eProps);
		if (qtractorTrack::midiBankSelMethod() >= 0) {
			pDocument->saveTextElement("midi-bank-sel-method",
				QString::number(qtractorTrack::midiBankSelMethod()), &eProps);
		}
		if (qtractorTrack::midiBank() >= 0) {
			pDocument->saveTextElement("midi-bank",
				QString::number(qtractorTrack::midiBank()), &eProps);
		}
		if (qtractorTrack::midiProgram() >= 0) {
			pDocument->saveTextElement("midi-program",
				QString::number(qtractorTrack::midiProgram()), &eProps);
		}
	}
	pElement->appendChild(eProps);

	// Save track state...
	QDomElement eState = pDocument->document()->createElement("state");
	pDocument->saveTextElement("mute",
		pDocument->textFromBool(qtractorTrack::isMute()), &eState);
	pDocument->saveTextElement("solo",
		pDocument->textFromBool(qtractorTrack::isSolo()), &eState);
	pDocument->saveTextElement("record",
		pDocument->textFromBool(qtractorTrack::isRecord()), &eState);
	pElement->appendChild(eState);

	// Save track view attributes...
	QDomElement eView = pDocument->document()->createElement("view");
	pDocument->saveTextElement("height",
		QString::number(qtractorTrack::height()), &eView);
	pDocument->saveTextElement("background-color",
		qtractorTrack::background().name(), &eView);
	pDocument->saveTextElement("foreground-color",
		qtractorTrack::foreground().name(), &eView);
	pElement->appendChild(eView);

	// Save track clips...
	QDomElement eClips = pDocument->document()->createElement("clips");
	for (qtractorClip *pClip = qtractorTrack::clips().first();
			pClip; pClip = pClip->next()) {
		// Create the new clip element...
		QDomElement eClip = pDocument->document()->createElement("clip");
		if (!pClip->saveElement(pDocument, &eClip))
			return false;
		// Add this clip...
		eClips.appendChild(eClip);
	}
	pElement->appendChild(eClips);

	return true;
}


// end of qtractorTrack.h
