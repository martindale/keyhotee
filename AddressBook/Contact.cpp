#include "Contact.hpp"

#include <bts/application.hpp>

#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>

/// QT headers
#include <QBuffer>

const QIcon& Contact::getIcon() const
  {
  return icon;
  }

Contact::Contact(const bts::addressbook::wallet_contact& contact)
  : bts::addressbook::wallet_contact(contact)
  {
  if (contact.icon_png.size() )
    {
      QString image_string = QString::fromLatin1(&contact.icon_png[0], contact.icon_png.size());
      QByteArray image_byte_array = image_string.toLatin1();

    QImage image;
    if (image.loadFromData( image_byte_array ) )
      icon = QIcon(QPixmap::fromImage(image) );
    else
      wlog("unable to load icon for contact ${c}", ("c", contact) );
    }
  else
    {
    icon.addFile(QStringLiteral(":/images/user.png"), QSize(), QIcon::Normal, QIcon::Off);
    }
  }

void Contact::setIcon(const QIcon& icon)
  {
  this->icon = icon;
  if (!icon.isNull() )
    {
      QImage     image(icon.pixmap(QSize(32, 32)).toImage());
    QByteArray byte_array;
    QBuffer    buffer(&byte_array);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");    // writes image into ba in PNG format

    icon_png.resize(byte_array.size() );
    memcpy(icon_png.data(), byte_array.data(), byte_array.size() );
    }
  else
    {
    icon_png.resize(0);
    }
  }

QString Contact::getEmail() const
  {
  return QString();
  }

void Contact::setEmail(const QString& email)
  {
  }

QString Contact::getPhone() const
  {
  return QString();
  }

void Contact::setPhone(const QString& phone)
  {
  }

QString Contact::getNotes() const
  {
  return QString(notes.c_str());
  }

void Contact::setNotes(const QString& phone)
  {
  notes = phone.toStdString();
  }

QString Contact::getLabel() const
  {
  QString label = (first_name + " " + last_name).c_str();
  if (label == " ")
    return dac_id_string.c_str();
  return label;
  }

bool Contact::isOwn() const
  {
  bts::application_ptr app = bts::application::instance();
  bts::profile_ptr     currentProfile = app->get_profile();
  bts::keychain        keyChain = currentProfile->get_keychain();

  typedef std::set<fc::ecc::public_key_data> TPublicKeyIndex;
  try
    {
    //put all public keys owned by profile into a set
    TPublicKeyIndex myPublicKeys;
    for (const auto& id : currentProfile->identities())
      {
      auto myPublicKey = keyChain.get_identity_key(id.dac_id_string).get_public_key();
      fc::ecc::public_key_data keyData = myPublicKey;
      myPublicKeys.insert(keyData);
      }
    //check if we have a public key in our set matching the contact's public key
    return myPublicKeys.find(public_key) != myPublicKeys.end();
    }
  catch (const fc::exception&)
    {
    return false;
    }
  }

int Contact::getAge() const
  {
  return getAge(*this);
  }

int Contact::getRepute() const
  {
  auto app = bts::application::instance();
  fc::optional<bts::bitname::name_record> oname_record =  app->lookup_name( dac_id_string );
  if (oname_record)
    {
    return oname_record->repute;
    }
  else
    return 0;
  }

int Contact::getAge(const bts::addressbook::contact& id)
  {
  auto app = bts::application::instance();
  /// FIXME reverse_name_lookup is not yet supported.
  bool usePK = false; /// id.dac_id_string.empty() && id.public_key.valid()
  fc::optional<bts::bitname::name_record> oname_record = usePK ?
    app->reverse_name_lookup(id.public_key) : app->lookup_name(id.dac_id_string);

  if (oname_record)
    {
    return oname_record->age;
    }
  else
    return 0;
  }

