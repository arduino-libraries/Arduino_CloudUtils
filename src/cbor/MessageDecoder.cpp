/*
    This file is part of the Arduino_CloudUtils library.

    Copyright (c) 2024 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "MessageDecoder.h"

MessageDecoder::Status CBORMessageDecoderSingleton::decode(Message* msg, const uint8_t* const buf, size_t &len) {
  // prepare cbor structure
  CborValue iter;
  CborTag tag;
  CborParser parser;

  if (cbor_parser_init(buf, len, 0, &parser, &iter) != CborNoError) {
    return MessageDecoder::Status::Error;
  }

  if (iter.type != CborTagType) {
    return MessageDecoder::Status::Error;
  }

  if (cbor_value_get_tag(&iter, &tag) != CborNoError) {
    return MessageDecoder::Status::Error;
  }

  if (cbor_value_advance(&iter) != CborNoError) {
    return MessageDecoder::Status::Error;
  }

  struct llnode* decoder_it = decoders;

  for(; decoder_it != nullptr; decoder_it = decoder_it->next) {
    if(decoder_it->tag == tag) {
      break;
    }
  }

  // check if message.id exists on the decoders list or return error
  if(decoder_it == nullptr) {
    return MessageDecoder::Status::Error;
  }

  // encode the message
  if(decoder_it->decoder->_decode(&iter, msg) == MessageDecoder::Status::Error) {
    return MessageDecoder::Status::Error;
  }

  return MessageDecoder::Status::Complete;
}

CBORMessageDecoderSingleton& CBORMessageDecoderSingleton::getInstance() {
  static CBORMessageDecoderSingleton singleton;

  return singleton;
}

void CBORMessageDecoderSingleton::append(CBORTag tag, CBORMessageDecoderInterface* decoder) {
  struct llnode* decoder_it = decoders;

  for(; decoder_it != nullptr; decoder_it = decoder_it->next) {
    if(decoder_it->tag == tag) {
      return;
    }
  }

  struct llnode* next = new llnode;

  next->tag = tag;
  next->decoder = decoder;

  // this is the first element
  if(decoders == nullptr) {
    decoders = next;
  } else {
    decoders_last->next = next;
  }

  decoders_last = next;
}

CBORMessageDecoderInterface::CBORMessageDecoderInterface(const CBORTag tag, const MessageId id)
: tag(tag), id(id) {
  // call singleton/global variable and insert this decoder
  CBORMessageDecoderSingleton::getInstance().append(tag, this);
}

MessageDecoder::Status CBORMessageDecoderInterface::_decode(CborValue* iter, Message *msg) {
  CborValue array_iter;
  msg->id = this->id;

  if (cbor_value_get_type(iter) != CborArrayType) {
    return MessageDecoder::Status::Error;
  }

  if (cbor_value_enter_container(iter, &array_iter) != CborNoError) {
    return MessageDecoder::Status::Error;
  }

  return decode(&array_iter, msg);
}
