// Code generated by Wire protocol buffer compiler, do not edit.
// Source file: wire.proto
package red.tel.chat.generated_protobuf;

import android.os.Parcelable;
import com.squareup.wire.AndroidMessage;
import com.squareup.wire.FieldEncoding;
import com.squareup.wire.Message;
import com.squareup.wire.ProtoAdapter;
import com.squareup.wire.ProtoReader;
import com.squareup.wire.ProtoWriter;
import com.squareup.wire.WireField;
import com.squareup.wire.internal.Internal;
import java.io.IOException;
import java.lang.Boolean;
import java.lang.Object;
import java.lang.Override;
import java.lang.String;
import java.lang.StringBuilder;
import okio.ByteString;

public final class Contact extends AndroidMessage<Contact, Contact.Builder> {
  public static final ProtoAdapter<Contact> ADAPTER = new ProtoAdapter_Contact();

  public static final Parcelable.Creator<Contact> CREATOR = AndroidMessage.newCreator(ADAPTER);

  private static final long serialVersionUID = 0L;

  public static final String DEFAULT_ID = "";

  public static final String DEFAULT_NAME = "";

  public static final Boolean DEFAULT_ONLINE = false;

  @WireField(
      tag = 1,
      adapter = "com.squareup.wire.ProtoAdapter#STRING"
  )
  public final String id;

  @WireField(
      tag = 2,
      adapter = "com.squareup.wire.ProtoAdapter#STRING"
  )
  public final String name;

  @WireField(
      tag = 3,
      adapter = "com.squareup.wire.ProtoAdapter#BOOL"
  )
  public final Boolean online;

  public Contact(String id, String name, Boolean online) {
    this(id, name, online, ByteString.EMPTY);
  }

  public Contact(String id, String name, Boolean online, ByteString unknownFields) {
    super(ADAPTER, unknownFields);
    this.id = id;
    this.name = name;
    this.online = online;
  }

  @Override
  public Builder newBuilder() {
    Builder builder = new Builder();
    builder.id = id;
    builder.name = name;
    builder.online = online;
    builder.addUnknownFields(unknownFields());
    return builder;
  }

  @Override
  public boolean equals(Object other) {
    if (other == this) return true;
    if (!(other instanceof Contact)) return false;
    Contact o = (Contact) other;
    return unknownFields().equals(o.unknownFields())
        && Internal.equals(id, o.id)
        && Internal.equals(name, o.name)
        && Internal.equals(online, o.online);
  }

  @Override
  public int hashCode() {
    int result = super.hashCode;
    if (result == 0) {
      result = unknownFields().hashCode();
      result = result * 37 + (id != null ? id.hashCode() : 0);
      result = result * 37 + (name != null ? name.hashCode() : 0);
      result = result * 37 + (online != null ? online.hashCode() : 0);
      super.hashCode = result;
    }
    return result;
  }

  @Override
  public String toString() {
    StringBuilder builder = new StringBuilder();
    if (id != null) builder.append(", id=").append(id);
    if (name != null) builder.append(", name=").append(name);
    if (online != null) builder.append(", online=").append(online);
    return builder.replace(0, 2, "Contact{").append('}').toString();
  }

  public static final class Builder extends Message.Builder<Contact, Builder> {
    public String id;

    public String name;

    public Boolean online;

    public Builder() {
    }

    public Builder id(String id) {
      this.id = id;
      return this;
    }

    public Builder name(String name) {
      this.name = name;
      return this;
    }

    public Builder online(Boolean online) {
      this.online = online;
      return this;
    }

    @Override
    public Contact build() {
      return new Contact(id, name, online, super.buildUnknownFields());
    }
  }

  private static final class ProtoAdapter_Contact extends ProtoAdapter<Contact> {
    public ProtoAdapter_Contact() {
      super(FieldEncoding.LENGTH_DELIMITED, Contact.class);
    }

    @Override
    public int encodedSize(Contact value) {
      return ProtoAdapter.STRING.encodedSizeWithTag(1, value.id)
          + ProtoAdapter.STRING.encodedSizeWithTag(2, value.name)
          + ProtoAdapter.BOOL.encodedSizeWithTag(3, value.online)
          + value.unknownFields().size();
    }

    @Override
    public void encode(ProtoWriter writer, Contact value) throws IOException {
      ProtoAdapter.STRING.encodeWithTag(writer, 1, value.id);
      ProtoAdapter.STRING.encodeWithTag(writer, 2, value.name);
      ProtoAdapter.BOOL.encodeWithTag(writer, 3, value.online);
      writer.writeBytes(value.unknownFields());
    }

    @Override
    public Contact decode(ProtoReader reader) throws IOException {
      Builder builder = new Builder();
      long token = reader.beginMessage();
      for (int tag; (tag = reader.nextTag()) != -1;) {
        switch (tag) {
          case 1: builder.id(ProtoAdapter.STRING.decode(reader)); break;
          case 2: builder.name(ProtoAdapter.STRING.decode(reader)); break;
          case 3: builder.online(ProtoAdapter.BOOL.decode(reader)); break;
          default: {
            FieldEncoding fieldEncoding = reader.peekFieldEncoding();
            Object value = fieldEncoding.rawProtoAdapter().decode(reader);
            builder.addUnknownField(tag, fieldEncoding, value);
          }
        }
      }
      reader.endMessage(token);
      return builder.build();
    }

    @Override
    public Contact redact(Contact value) {
      Builder builder = value.newBuilder();
      builder.clearUnknownFields();
      return builder.build();
    }
  }
}
