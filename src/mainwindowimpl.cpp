/**
 * \file mainwindowimpl.cpp
 * \brief Implementation of the main window
 * \author Diego Iastrubni elcuco@kde.org
 * License GPLv3, 2008,2009
 */

#include <QMessageBox>
#include <QStatusBar>
#include <QFileDialog>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QDebug>

#include "qwt_plot_curve.h"
#include "qwt_symbol.h"
#include "dataset.h"
#include "datasetview.h"
#include "nodeplotcurve.h"
#include "mainwindowimpl.h"

// size of the input table
#define SIZE_X			256
#define SIZE_Y			2*2024

// this is in millisecconds
#define UPDATE_TIMER		50
#define MESSAGE_TIMEOUT		5000

// this controls how much items will be 
// created, and their range
#define MAX_RANDOM_NODES	1000
#define MAX_VALUE		1000


MainWindowImpl::MainWindowImpl( QWidget * parent, Qt::WFlags f) 
	: QMainWindow(parent, f)
{
	// don't bee n00b, initialize the random seeder
	srand( (int) winId() );
	
	setupUi(this);
	updateTimer = 0;
	connect( pushButton, SIGNAL(clicked()), this, SLOT(on_actionProcess_triggered()) ); 

	QStringList l;
	tableWidget->setRowCount( SIZE_Y );
	tableWidget->setColumnCount( SIZE_X );	
	for( int x=0; x<SIZE_X; x++ )
	{
		l << QString::number( x+1, 16 ).toUpper();
		tableWidget->columnWidth( 25 ); 
	}
	tableWidget->setHorizontalHeaderLabels(l);

	QwtSymbol symbol;
	symbol.setStyle(QwtSymbol::XCross);
	symbol.setSize(QSize(6, 6));

	kmeansPlot->setCanvasBackground(Qt::black);
	kmeansDataSet = new DataSet();
	kmeansDataSetView = new DataSetView( kmeansDataSet );
	kmeansPlotCurve = new NodesPlotCurve("KMeans");
	kmeansPlotCurve->setStyle(QwtPlotCurve::NoCurve);
	kmeansPlotCurve->setSymbol(symbol);
	kmeansPlotCurve->attach( kmeansPlot );	
	
	claraPlot->setCanvasBackground(Qt::black);
	claraDataSet = new DataSet();
	claraDataSetView = new DataSetView( claraDataSet );
	claraPlotCurve = new NodesPlotCurve("Clara");
	claraPlotCurve->setStyle(QwtPlotCurve::NoCurve);
	claraPlotCurve->setSymbol(symbol);
	claraPlotCurve->attach( claraPlot );
	
	pamPlot->setCanvasBackground(Qt::black);
	pamDataSet = new DataSet();
	pamDataSetView = new DataSetView( pamDataSet );
	pamPlotCurve = new NodesPlotCurve("PAM");
	pamPlotCurve->setStyle(QwtPlotCurve::NoCurve);
	pamPlotCurve->setSymbol(symbol);
	pamPlotCurve->attach( pamPlot );
		
	statusBar()->showMessage( tr("Welcome"), MESSAGE_TIMEOUT );
}

void MainWindowImpl::setRandomData( int dimentions )
{
        stopTheTimer();

        tableWidget->clear();
	for( int x=0; x<MAX_RANDOM_NODES; x++ )
		for ( int y=0; y<dimentions; y++ )
			tableWidget->setItem( x, y, new QTableWidgetItem(QString::number(rand()%MAX_VALUE)));
	
	on_actionProcess_triggered();
	statusBar()->showMessage( tr("Random data has been defined"), 5000 );
}

void MainWindowImpl::populateDataSet()
{
	//if (kmeansCheckBox->isChecked())
		kmeansDataSet->clear();
	//if (claraCheckBox->isChecked())
		claraDataSet->clear();
	//if (pamCheckBox->isChecked())
		pamDataSet->clear();
	
	for ( int n=0; n<SIZE_Y; n++ )
	{
		QTableWidgetItem *itemX = tableWidget->item(n,0);
		QTableWidgetItem *itemY = tableWidget->item(n,1);
		bool ok;
		
		if (!itemX) continue;
		if (!itemY) continue;

		double x = itemX->text().toDouble(&ok);		
		if (!ok) continue;
		double y = itemY->text().toDouble(&ok);
		if (!ok) continue;

		//if (kmeansCheckBox->isChecked())
			kmeansDataSet->addItem(x,y);
		//if (claraCheckBox->isChecked())
			claraDataSet->addItem(x,y);
		//if (pamCheckBox->isChecked())
			pamDataSet->addItem(x,y);
	}
}

void MainWindowImpl::stopTheTimer()
{
        if (updateTimer!=0)
                killTimer( updateTimer );
        updateTimer = 0;
}


void MainWindowImpl::on_actionLoad_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open Image"), "", tr("All files (*)")
	);
	
	if (fileName.isEmpty())
	{
		statusBar()->showMessage( tr("File load aborted"), MESSAGE_TIMEOUT );
		return;
	}
	
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		statusBar()->showMessage( tr("Could not open file: %1").arg(fileName), MESSAGE_TIMEOUT );
		return;
	}
	
        if (updateTimer!=0)
                killTimer( updateTimer );
        int y = 0;
	while (!file.atEnd()) 
	{
		QString line = file.readLine().simplified();
		
		if (line.startsWith('#') || (line.isEmpty()) )
			continue;
			
		//QStringList	coordinates = line.split(QRegExp("\\W+"), QString::SkipEmptyParts);
		QStringList	coordinates = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
		int x = 0;
		
		foreach( QString s, coordinates )
		{
			tableWidget->setItem( y, x, new QTableWidgetItem(s));
			x++;
		}
		y++;
	};

	on_actionProcess_triggered();
	//dataTab->setCurrentIndex( 2 );
	statusBar()->showMessage( tr("File %1 loaded").arg(fileName), MESSAGE_TIMEOUT );
}

void MainWindowImpl::on_actionClearDataSet_triggered()
{
        stopTheTimer();
	tableWidget->clear();
	claraDataSet->clear();
	kmeansDataSet->clear();
	pamDataSet->clear();
	on_actionProcess_triggered();
	statusBar()->showMessage( tr("Dataset cleared"), MESSAGE_TIMEOUT );
}


/// switch between the KMeans/Clara/PAM displays
void MainWindowImpl::on_actionSwitchView_triggered()
{
	int i = (dataTab->currentIndex() + 1) % 4;
	dataTab->setCurrentIndex( i );
}

void MainWindowImpl::on_actionRandom2D_triggered()
{
	setRandomData( 2 );
}

void MainWindowImpl::on_actionRandom3D_triggered()
{
	setRandomData( 3 );
}

void MainWindowImpl::on_actionProcess_triggered()
{
        stopTheTimer();
	populateDataSet();
	
	//if (kmeansCheckBox->isChecked())
	{
		kmeansDataSet->KMeans_init( centroidsNumber->value() );
		kmeansPlotCurve->attachToDataSet( kmeansDataSetView );
		kmeansPlot->replot();
	}
	
	//if (claraCheckBox->isChecked())
	{
		claraDataSet->Clara_init( centroidsNumber->value() );
		claraPlotCurve->attachToDataSet( claraDataSetView ); 
		claraPlot->replot();		
	}
	
	//if (pamCheckBox->isChecked())
	{
		pamDataSet->PAM_init( centroidsNumber->value() );
		pamPlotCurve->attachToDataSet( pamDataSetView ); 
		pamPlot->replot();
	}
	
	if ( (claraDataSet->getItemCount()!=0) || (pamDataSet->getItemCount() != 0))
		updateTimer = startTimer( UPDATE_TIMER );
}

void MainWindowImpl::timerEvent(QTimerEvent *event)
{
	if (kmeansCheckBox->isChecked())
	{
		kmeansDataSet->KMeans_calculateNewCentroids();
		kmeansDataSet->calculateAssociations();
		kmeansPlotCurve->attachToDataSet( kmeansDataSetView ); 
		kmeansPlot->replot();
        }

	if (claraCheckBox->isChecked())
	{
		claraDataSet->Clara_calculateNewCentroids();
		claraDataSet->calculateAssociations();
		claraPlotCurve->attachToDataSet( claraDataSetView ); 
		claraPlot->replot();
	}
	
	if (pamCheckBox->isChecked())
	{
		pamDataSet->PAM_calculateNewCentroids();
		pamDataSet->calculateAssociations();
		pamPlotCurve->attachToDataSet( pamDataSetView ); 
		pamPlot->replot();
        }

        Q_UNUSED(event);
}

void MainWindowImpl::on_actionAbout_triggered()
{
	QMessageBox::information( this, "About", 
                "Data Mining application, KMeans/PAM/Clara demo\n"
		"\n"
		"  Diego Iastrubni - diegoiast@gmail.com\n"
	);
}
