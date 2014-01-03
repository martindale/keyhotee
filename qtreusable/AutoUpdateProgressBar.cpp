#include "qtreusable/AutoUpdateProgressBar.hpp"

#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>

/// Helper notifier function, needed to transmit progress update signals between threads.
class TUpdateNotifier : public QObject
  {
  Q_OBJECT
  public:
    virtual ~TUpdateNotifier() {}

    /// Should be called to trigger progress bar update
    void updateProgress(int value)
      {
      emit onUpdateProgress(value);
      }

  /// The client code should connect to this signal to make progress update safely.
    Q_SIGNALS: void onUpdateProgress(int value);
  };

TAutoUpdateProgressBar* 
TAutoUpdateProgressBar::create(const QRect& rect, const QString& title, unsigned int max,
  QWidget* parent /*= nullptr*/)
  {
  TAutoUpdateProgressBar* bar = new TAutoUpdateProgressBar(max, parent);
  bar->setWindowTitle(title);
  bar->setWindowFlags(Qt::WindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint));

  bar->_notifier = new TUpdateNotifier();

  connect(bar->_notifier, SIGNAL(onUpdateProgress(int)), bar, SLOT(setValue(int)), Qt::BlockingQueuedConnection);

  connect(&bar->_futureWatcher, SIGNAL(finished()), bar, SLOT(onFinish()));

  bar->move(rect.topLeft());
  bar->resize(rect.size());
  bar->show();

  return bar;
  }

void TAutoUpdateProgressBar::doTask(std::function<void()> mainTask, std::function<void()> onFinish)
  {
  _onFinishAction = onFinish;
  QFuture<void> f = QtConcurrent::run(mainTask);
  _futureWatcher.setFuture(f);
  }

void TAutoUpdateProgressBar::release()
  {
  hide();
  deleteLater();
  }

void TAutoUpdateProgressBar::updateValue(int value)
  {
  _notifier->updateProgress(value);
  }

TAutoUpdateProgressBar::TAutoUpdateProgressBar(unsigned int maxValue, QWidget* parent) :
  QProgressBar(parent),
  _maxValue(maxValue)
  {
  setMaximum(maxValue);
  }

TAutoUpdateProgressBar::~TAutoUpdateProgressBar()
  {
  _notifier->deleteLater();
  }

void TAutoUpdateProgressBar::onFinish()
  {
  setValue(_maxValue);
  release();
  _onFinishAction();
  }

#include "AutoUpdateProgressBar.moc"
