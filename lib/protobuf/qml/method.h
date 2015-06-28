#ifndef PROTOBUF_QML_METHOD_H
#define PROTOBUF_QML_METHOD_H

#include "protobuf/qml/common.h"
#include "protobuf/qml/descriptors.h"
#include <QObject>

namespace protobuf {
namespace qml {

class Channel2;

class PROTOBUF_QML_DLLEXPORT MethodBase : public QObject {
  Q_OBJECT

signals:
  void data(int tag, const QVariant& data);
  void error(int tag, const QString& message);
  void closed(int tag);

protected:
  explicit MethodBase(QObject* p = nullptr) : QObject(p) {}
  virtual ~MethodBase() {}
};

class PROTOBUF_QML_DLLEXPORT MethodHolder : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString methodName READ method_name WRITE set_method_name NOTIFY
                 methodNameChanged)

  Q_PROPERTY(protobuf::qml::DescriptorWrapper* readDescriptor READ
                 read_descriptor WRITE set_read_descriptor NOTIFY
                     readDescriptorChanged)
  Q_PROPERTY(protobuf::qml::DescriptorWrapper* writeDescriptor READ
                 write_descriptor WRITE set_write_descriptor NOTIFY
                     writeDescriptorChanged)

signals:
  void data(int tag, const QVariant& data);
  void error(int tag, const QString& message);
  void closed(int tag);

  void methodNameChanged();
  void readDescriptorChanged();
  void writeDescriptorChanged();
  // void implChanged();

public:
  explicit MethodHolder(QObject* p = nullptr) : QObject(p) {}
  virtual ~MethodHolder() {}

  const QString& method_name() const { return method_name_; }
  void set_method_name(const QString& method_name) {
    if (method_name_ != method_name) {
      deinit();
      method_name_ = method_name;
      methodNameChanged();
    }
  }

  DescriptorWrapper* read_descriptor() const { return read_desc_; }
  void set_read_descriptor(DescriptorWrapper* read_desc) {
    if (read_desc != read_desc_) {
      deinit();
      read_desc_ = read_desc;
      readDescriptorChanged();
    }
  }

  DescriptorWrapper* write_descriptor() const { return write_desc_; }
  void set_write_descriptor(DescriptorWrapper* write_desc) {
    if (write_desc != write_desc_) {
      deinit();
      write_desc_ = write_desc;
      writeDescriptorChanged();
    }
  }

protected:
  virtual void deinit() {}

// private:
  QString method_name_;
  DescriptorWrapper* read_desc_ = nullptr;
  DescriptorWrapper* write_desc_ = nullptr;
};

class ClientMethodHolder : public MethodHolder {
  Q_OBJECT

  Q_PROPERTY(protobuf::qml::Channel2* channel READ channel WRITE set_channel
                 NOTIFY channelChanged)

signals:
  void channelChanged();

public:
  explicit ClientMethodHolder(QObject* p = nullptr) : MethodHolder(p) {}
  virtual ~ClientMethodHolder() {}

  Channel2* channel() const { return channel_; }
  void set_channel(Channel2* channel) {
    if (channel != channel_) {
      deinit();
      channel_ = channel;
      channelChanged();
    }
  }

protected:
  bool readyForInit() {
    return channel_ && !method_name_.isEmpty() && read_desc_ && write_desc_;
  }

private:
  Channel2* channel_ = nullptr;
};

class PROTOBUF_QML_DLLEXPORT UnaryMethod : public MethodBase {
  Q_OBJECT

public:
  explicit UnaryMethod(QObject* p = nullptr) : MethodBase(p) {}
  virtual ~UnaryMethod() {}
  virtual bool write(int tag, const QVariant& data, int timeout) {
    return false;
  }
};

class PROTOBUF_QML_DLLEXPORT UnaryMethodHolder : public ClientMethodHolder {
  Q_OBJECT

public:
  explicit UnaryMethodHolder(QObject* p = nullptr) : ClientMethodHolder(p) {}
  ~UnaryMethodHolder() {}

  Q_INVOKABLE bool write(int tag, const QVariant& data, int timeout) {
    if (!ensureInit()) {
      qWarning() << "Failed to initialize unary method implementation.";
      return false;
    }
    return impl_->write(tag, data, timeout);
  }

protected:
  void deinit() final {
    // TODO: Is it safe at any time ??
    impl_.reset();
  }

private:
  bool ensureInit();

  std::unique_ptr<UnaryMethod> impl_;
};

class PROTOBUF_QML_DLLEXPORT WriterMethod : public MethodBase {
  Q_OBJECT

public:
  explicit WriterMethod(QObject* p = nullptr) : MethodBase(p) {}
  virtual ~WriterMethod() {}
  virtual bool write(int tag, const QVariant& data) { return false; }
  virtual bool writesDone(int tag) { return false; }
  virtual int timeout(int tag) const { return -1; }
  virtual void set_timeout(int tag, int milliseconds) {}
};

class PROTOBUF_QML_DLLEXPORT WriterMethodHolder : public ClientMethodHolder {
  Q_OBJECT

public:
  explicit WriterMethodHolder(QObject* p = nullptr) : ClientMethodHolder(p) {}
  ~WriterMethodHolder() {}

  Q_INVOKABLE bool write(int tag, const QVariant& data, int timeout) {
    if (!ensureInit()) {
      qWarning() << "Failed to initialize writer method implementation.";
      return false;
    }
    return impl_->write(tag, data);
  }

  Q_INVOKABLE bool writesDone(int tag, int timeout) {
    if (!ensureInit()) {
      qWarning() << "Failed to initialize writer method implementation.";
      return false;
    }
    return impl_->writesDone(tag);
  }

  Q_INVOKABLE int timeout(int tag) {
    if (!ensureInit()) {
      qWarning() << "Failed to initialize unary method implementation.";
      return -1;
    }
    return impl_->timeout(tag);
  }
  Q_INVOKABLE void set_timeout(int tag, int milliseconds) {
    if (!ensureInit()) {
      qWarning() << "Failed to initialize unary method implementation.";
      return;
    }
    impl_->set_timeout(tag, milliseconds);
  }

protected:
  void deinit() final {
    // TODO: Is it safe at any time ??
    impl_.reset();
  }

private:
  bool ensureInit();

  std::unique_ptr<WriterMethod> impl_;
};

class PROTOBUF_QML_DLLEXPORT ReaderMethod : public MethodBase {
  Q_OBJECT

signals:
  void dataEnd(int tag);

public:
  explicit ReaderMethod(QObject* p = nullptr) : MethodBase(p) {}
  virtual ~ReaderMethod() {}
};

class PROTOBUF_QML_DLLEXPORT ReaderWriterMethod : public MethodBase {
  Q_OBJECT

signals:
  void dataEnd(int tag);

public:
  explicit ReaderWriterMethod(QObject* p = nullptr) : MethodBase(p) {}
  virtual ~ReaderWriterMethod() {}
  virtual bool writesDone(int tag) { return false; }
};

class PROTOBUF_QML_DLLEXPORT Channel2 : public QObject {
  Q_OBJECT

public:
  explicit Channel2(QObject* p = nullptr) : QObject(p) {}

  virtual ~Channel2() {}

  virtual UnaryMethod* registerUnaryMethod(const QString& name,
                                           DescriptorWrapper* read,
                                           DescriptorWrapper* write) {
    return nullptr;
  }

  virtual ReaderMethod* registerReaderMethod(const QString& name,
                                             DescriptorWrapper* read,
                                             DescriptorWrapper* write) {
    return nullptr;
  }

  virtual WriterMethod* registerWriterMethod(const QString& name,
                                             DescriptorWrapper* read,
                                             DescriptorWrapper* write) {
    return nullptr;
  }

  virtual ReaderWriterMethod* registerReaderWriterMethod(
      const QString& name, DescriptorWrapper* read, DescriptorWrapper* write) {
    return nullptr;
  }
};
}
}
#endif  // PROTOBUF_QML_METHOD_H
