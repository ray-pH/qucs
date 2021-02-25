/***************************************************************************
    copyright            : (C) 2021 by Felix Salfelder
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// provide access to "dat" files in a given directory
/* -------------------------------------------------------------------------------- */
#include "data.h"
#include "language.h"
#include "output.h"
#include "qio.h"
#include "qt_compat.h"
#include "qucs_globals.h"
/* -------------------------------------------------------------------------------- */
class DatFiles : public Data{
public:
	explicit DatFiles() : Data(){itested();
		setLabel("datfile");
	}
	DatFiles(DatFiles const&d) : Data(d){ }

private:
	DatFiles* clone() const override { return new DatFiles(*this); }

	void set_param_by_name(std::string const& n, std::string const& v) override{
		if(n=="$schematic_filename"){
			_file_name = v;
		}else{ untested();
			incomplete();
		}
	}

private: // Element. remove?
	void paint(ViewPainter*) const {incomplete();}

private: // Data
	void prepare() override{ incomplete(); }
	void refresh() const override;

private:
	std::string _file_name;
}f1;
static Dispatcher<Data>::INSTALL d1(&dataDispatcher, "datfiles", &f1);
/* -------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------- */
class DatListing : public SimOutputDir{
public:
	explicit DatListing(std::string const& dir, QStringList const& Elements){
		auto lang = languageDispatcher["qucsator"];
		assert(lang);

		for(auto DataSet : Elements){
			auto dat = dataDispatcher.clone("datfile"); // really? table?
			assert(dat);
			std::string fn = dir + "/" + DataSet.toStdString();
			trace1("datlisting prepare", fn);
			dat->set_param_by_name("filename", fn); // really?
			auto l = DataSet.size();
			dat->setLabel(DataSet.toStdString().substr(0, l-4));
			assert(dat->label()!="");

			try{
				istream_t cs(istream_t::_WHOLE_FILE, fn);
				lang->parseItem(cs, dat);
				assert(dat->label()!="");
				assert(dat->common()->label()!="");
				push_back(dat->common());
			}catch(qucs::Exception_File_Open const&){
				message(QucsMsgWarning, "cannot load " + fn);
			}
			delete dat;
		}
	}
};
/* -------------------------------------------------------------------------------- */
void DatFiles::refresh() const
{
	trace1("DatFiles::refresh", _file_name);
	QFileInfo Info(QString_(_file_name));
	QDir ProjDir(Info.path());
	QStringList Elements = ProjDir.entryList(QStringList("*.dat"), QDir::Files, QDir::Name);

	DatListing* L = new DatListing(ProjDir.absolutePath().toStdString(), Elements);
	L->setLabel("datfiles");
	attach(L);
}
/* -------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------- */
