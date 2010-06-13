#include <QDebug>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <QApplication>
#include <kaboutdata.h>
#include <kcomponentdata.h>

int main(int argc, char **argv)
{
    KAboutData about( "permissions", "permissions", ki18n("permissions test"), "0.1" );
    KComponentData comonentData(&about);
    QApplication app(argc, argv);

    QStringList args;
    for (int i=1; i<argc; ++i) {
        args << argv[i];
    }
    if (args.count() != 2) {
        qFatal("Required parameters: source destination");
    }
    qDebug() << KUrl(args[0]) << KUrl(args[1]);
    KIO::FileCopyJob* job = KIO::file_copy(KUrl(args[0]), KUrl(args[1]), -1, KIO::Overwrite | KIO::HideProgressInfo);
    if (KIO::NetAccess::synchronousRun(job, 0)) {
        qDebug() << "successfully copied";
        return 0;
    } else {
        qDebug() << "copy failed";
        return 1;
    }
    return 0;
}
