/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <limits>
#include <type_traits>

#include <system/audio.h>

#include <android-base/expected.h>

#include <android/media/AudioAttributesInternal.h>
#include <android/media/AudioClient.h>
#include <android/media/AudioConfig.h>
#include <android/media/AudioConfigBase.h>
#include <android/media/AudioFlag.h>
#include <android/media/AudioGainMode.h>
#include <android/media/AudioInputFlags.h>
#include <android/media/AudioIoConfigEvent.h>
#include <android/media/AudioIoDescriptor.h>
#include <android/media/AudioOutputFlags.h>
#include <android/media/AudioPortConfigType.h>
#include <android/media/AudioTimestampInternal.h>
#include <android/media/EffectDescriptor.h>

#include <android/media/SharedFileRegion.h>
#include <binder/IMemory.h>
#include <media/AudioClient.h>
#include <media/AudioIoDescriptor.h>
#include <media/AudioTimestamp.h>
#include <system/audio_effect.h>

namespace android {

template <typename T>
using ConversionResult = base::expected<T, status_t>;

// Convenience macros for working with ConversionResult, useful for writing converted for aggregate
// types.

#define VALUE_OR_RETURN(result)                                \
    ({                                                         \
        auto _tmp = (result);                                  \
        if (!_tmp.ok()) return base::unexpected(_tmp.error()); \
        std::move(_tmp.value());                               \
    })

#define RETURN_IF_ERROR(result) \
    if (status_t _tmp = (result); _tmp != OK) return base::unexpected(_tmp);

/**
 * A generic template to safely cast between integral types, respecting limits of the destination
 * type.
 */
template<typename To, typename From>
ConversionResult<To> convertIntegral(From from) {
    // Special handling is required for signed / vs. unsigned comparisons, since otherwise we may
    // have the signed converted to unsigned and produce wrong results.
    if (std::is_signed_v<From> && !std::is_signed_v<To>) {
        if (from < 0 || from > std::numeric_limits<To>::max()) {
            return base::unexpected(BAD_VALUE);
        }
    } else if (std::is_signed_v<To> && !std::is_signed_v<From>) {
        if (from > std::numeric_limits<To>::max()) {
            return base::unexpected(BAD_VALUE);
        }
    } else {
        if (from < std::numeric_limits<To>::min() || from > std::numeric_limits<To>::max()) {
            return base::unexpected(BAD_VALUE);
        }
    }
    return static_cast<To>(from);
}

/**
 * A generic template to safely cast between types, that are intended to be the same size, but
 * interpreted differently.
 */
template<typename To, typename From>
ConversionResult<To> convertReinterpret(From from) {
    static_assert(sizeof(From) == sizeof(To));
    return static_cast<To>(from);
}

// maxSize is the size of the C-string buffer (including the 0-terminator), NOT the max length of
// the string.
status_t aidl2legacy_string(std::string_view aidl, char* dest, size_t maxSize);
ConversionResult<std::string> legacy2aidl_string(const char* legacy, size_t maxSize);

ConversionResult<audio_module_handle_t> aidl2legacy_int32_t_audio_module_handle_t(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_audio_module_handle_t_int32_t(audio_module_handle_t legacy);

ConversionResult<audio_io_handle_t> aidl2legacy_int32_t_audio_io_handle_t(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_audio_io_handle_t_int32_t(audio_io_handle_t legacy);

ConversionResult<audio_port_handle_t> aidl2legacy_int32_t_audio_port_handle_t(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_audio_port_handle_t_int32_t(audio_port_handle_t legacy);

ConversionResult<audio_patch_handle_t> aidl2legacy_int32_t_audio_patch_handle_t(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_audio_patch_handle_t_int32_t(audio_patch_handle_t legacy);

ConversionResult<audio_unique_id_t> aidl2legacy_int32_t_audio_unique_id_t(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_audio_unique_id_t_int32_t(audio_unique_id_t legacy);

// The legacy enum is unnamed. Thus, we use int.
ConversionResult<int> aidl2legacy_AudioPortConfigType(media::AudioPortConfigType aidl);
// The legacy enum is unnamed. Thus, we use int.
ConversionResult<media::AudioPortConfigType> legacy2aidl_AudioPortConfigType(int legacy);

ConversionResult<unsigned int> aidl2legacy_int32_t_config_mask(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_config_mask_int32_t(unsigned int legacy);

ConversionResult<audio_channel_mask_t> aidl2legacy_int32_t_audio_channel_mask_t(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_audio_channel_mask_t_int32_t(audio_channel_mask_t legacy);

ConversionResult<pid_t> aidl2legacy_int32_t_pid_t(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_pid_t_int32_t(pid_t legacy);

ConversionResult<uid_t> aidl2legacy_int32_t_uid_t(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_uid_t_int32_t(uid_t legacy);

ConversionResult<String8> aidl2legacy_string_view_String8(std::string_view aidl);
ConversionResult<std::string> legacy2aidl_String8_string(const String8& legacy);

ConversionResult<String16> aidl2legacy_string_view_String16(std::string_view aidl);
ConversionResult<std::string> legacy2aidl_String16_string(const String16& legacy);

ConversionResult<audio_io_config_event> aidl2legacy_AudioIoConfigEvent_audio_io_config_event(
        media::AudioIoConfigEvent aidl);
ConversionResult<media::AudioIoConfigEvent> legacy2aidl_audio_io_config_event_AudioIoConfigEvent(
        audio_io_config_event legacy);

ConversionResult<audio_port_role_t> aidl2legacy_AudioPortRole_audio_port_role_t(
        media::AudioPortRole aidl);
ConversionResult<media::AudioPortRole> legacy2aidl_audio_port_role_t_AudioPortRole(
        audio_port_role_t legacy);

ConversionResult<audio_port_type_t> aidl2legacy_AudioPortType_audio_port_type_t(
        media::AudioPortType aidl);
ConversionResult<media::AudioPortType> legacy2aidl_audio_port_type_t_AudioPortType(
        audio_port_type_t legacy);

ConversionResult<audio_format_t> aidl2legacy_AudioFormat_audio_format_t(
        media::audio::common::AudioFormat aidl);
ConversionResult<media::audio::common::AudioFormat> legacy2aidl_audio_format_t_AudioFormat(
        audio_format_t legacy);

ConversionResult<int> aidl2legacy_AudioGainMode_int(media::AudioGainMode aidl);
ConversionResult<media::AudioGainMode> legacy2aidl_int_AudioGainMode(int legacy);

ConversionResult<audio_gain_mode_t> aidl2legacy_int32_t_audio_gain_mode_t(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_audio_gain_mode_t_int32_t(audio_gain_mode_t legacy);

ConversionResult<audio_devices_t> aidl2legacy_int32_t_audio_devices_t(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_audio_devices_t_int32_t(audio_devices_t legacy);

ConversionResult<audio_gain_config> aidl2legacy_AudioGainConfig_audio_gain_config(
        const media::AudioGainConfig& aidl, media::AudioPortRole role, media::AudioPortType type);
ConversionResult<media::AudioGainConfig> legacy2aidl_audio_gain_config_AudioGainConfig(
        const audio_gain_config& legacy, audio_port_role_t role, audio_port_type_t type);

ConversionResult<audio_input_flags_t> aidl2legacy_AudioInputFlags_audio_input_flags_t(
        media::AudioInputFlags aidl);
ConversionResult<media::AudioInputFlags> legacy2aidl_audio_input_flags_t_AudioInputFlags(
        audio_input_flags_t legacy);

ConversionResult<audio_output_flags_t> aidl2legacy_AudioOutputFlags_audio_output_flags_t(
        media::AudioOutputFlags aidl);
ConversionResult<media::AudioOutputFlags> legacy2aidl_audio_output_flags_t_AudioOutputFlags(
        audio_output_flags_t legacy);

ConversionResult<audio_input_flags_t> aidl2legacy_audio_input_flags_mask(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_audio_input_flags_mask(audio_input_flags_t legacy);

ConversionResult<audio_output_flags_t> aidl2legacy_audio_output_flags_mask(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_audio_output_flags_mask(audio_output_flags_t legacy);

ConversionResult<audio_io_flags> aidl2legacy_AudioIoFlags_audio_io_flags(
        const media::AudioIoFlags& aidl, media::AudioPortRole role, media::AudioPortType type);
ConversionResult<media::AudioIoFlags> legacy2aidl_audio_io_flags_AudioIoFlags(
        const audio_io_flags& legacy, audio_port_role_t role, audio_port_type_t type);

ConversionResult<audio_port_config_device_ext> aidl2legacy_AudioPortConfigDeviceExt(
        const media::AudioPortConfigDeviceExt& aidl);
ConversionResult<media::AudioPortConfigDeviceExt> legacy2aidl_AudioPortConfigDeviceExt(
        const audio_port_config_device_ext& legacy);

ConversionResult<audio_stream_type_t> aidl2legacy_AudioStreamType_audio_stream_type_t(
        media::AudioStreamType aidl);
ConversionResult<media::AudioStreamType> legacy2aidl_audio_stream_type_t_AudioStreamType(
        audio_stream_type_t legacy);

ConversionResult<audio_source_t> aidl2legacy_AudioSourceType_audio_source_t(
        media::AudioSourceType aidl);
ConversionResult<media::AudioSourceType> legacy2aidl_audio_source_t_AudioSourceType(
        audio_source_t legacy);

ConversionResult<audio_session_t> aidl2legacy_int32_t_audio_session_t(int32_t aidl);
ConversionResult<int32_t> legacy2aidl_audio_session_t_int32_t(audio_session_t legacy);

ConversionResult<audio_port_config_mix_ext> aidl2legacy_AudioPortConfigMixExt(
        const media::AudioPortConfigMixExt& aidl, media::AudioPortRole role);
ConversionResult<media::AudioPortConfigMixExt> legacy2aidl_AudioPortConfigMixExt(
        const audio_port_config_mix_ext& legacy, audio_port_role_t role);

ConversionResult<audio_port_config_session_ext> aidl2legacy_AudioPortConfigSessionExt(
        const media::AudioPortConfigSessionExt& aidl);
ConversionResult<media::AudioPortConfigSessionExt> legacy2aidl_AudioPortConfigSessionExt(
        const audio_port_config_session_ext& legacy);

ConversionResult<audio_port_config> aidl2legacy_AudioPortConfig_audio_port_config(
        const media::AudioPortConfig& aidl);
ConversionResult<media::AudioPortConfig> legacy2aidl_audio_port_config_AudioPortConfig(
        const audio_port_config& legacy);

ConversionResult<struct audio_patch> aidl2legacy_AudioPatch_audio_patch(
        const media::AudioPatch& aidl);
ConversionResult<media::AudioPatch> legacy2aidl_audio_patch_AudioPatch(
        const struct audio_patch& legacy);

ConversionResult<sp<AudioIoDescriptor>> aidl2legacy_AudioIoDescriptor_AudioIoDescriptor(
        const media::AudioIoDescriptor& aidl);

ConversionResult<media::AudioIoDescriptor> legacy2aidl_AudioIoDescriptor_AudioIoDescriptor(
        const sp<AudioIoDescriptor>& legacy);

ConversionResult<AudioClient> aidl2legacy_AudioClient(const media::AudioClient& aidl);
ConversionResult<media::AudioClient> legacy2aidl_AudioClient(const AudioClient& legacy);

ConversionResult<audio_content_type_t>
aidl2legacy_AudioContentType_audio_content_type_t(media::AudioContentType aidl);
ConversionResult<media::AudioContentType>
legacy2aidl_audio_content_type_t_AudioContentType(audio_content_type_t legacy);

ConversionResult<audio_usage_t>
aidl2legacy_AudioUsage_audio_usage_t(media::AudioUsage aidl);
ConversionResult<media::AudioUsage>
legacy2aidl_audio_usage_t_AudioUsage(audio_usage_t legacy);

ConversionResult<audio_flags_mask_t>
aidl2legacy_AudioFlag_audio_flags_mask_t(media::AudioFlag aidl);
ConversionResult<media::AudioFlag>
legacy2aidl_audio_flags_mask_t_AudioFlag(audio_flags_mask_t legacy);

ConversionResult<audio_flags_mask_t>
aidl2legacy_int32_t_audio_flags_mask_t_mask(int32_t aidl);
ConversionResult<int32_t>
legacy2aidl_audio_flags_mask_t_int32_t_mask(audio_flags_mask_t legacy);

ConversionResult<audio_attributes_t>
aidl2legacy_AudioAttributesInternal_audio_attributes_t(const media::AudioAttributesInternal& aidl);
ConversionResult<media::AudioAttributesInternal>
legacy2aidl_audio_attributes_t_AudioAttributesInternal(const audio_attributes_t& legacy);

ConversionResult<audio_encapsulation_mode_t>
aidl2legacy_audio_encapsulation_mode_t_AudioEncapsulationMode(media::AudioEncapsulationMode aidl);
ConversionResult<media::AudioEncapsulationMode>
legacy2aidl_AudioEncapsulationMode_audio_encapsulation_mode_t(audio_encapsulation_mode_t legacy);

ConversionResult<audio_offload_info_t>
aidl2legacy_AudioOffloadInfo_audio_offload_info_t(const media::AudioOffloadInfo& aidl);
ConversionResult<media::AudioOffloadInfo>
legacy2aidl_audio_offload_info_t_AudioOffloadInfo(const audio_offload_info_t& legacy);

ConversionResult<audio_config_t>
aidl2legacy_AudioConfig_audio_config_t(const media::AudioConfig& aidl);
ConversionResult<media::AudioConfig>
legacy2aidl_audio_config_t_AudioConfig(const audio_config_t& legacy);

ConversionResult<audio_config_base_t>
aidl2legacy_AudioConfigBase_audio_config_base_t(const media::AudioConfigBase& aidl);
ConversionResult<media::AudioConfigBase>
legacy2aidl_audio_config_base_t_AudioConfigBase(const audio_config_base_t& legacy);

ConversionResult<sp<IMemory>>
aidl2legacy_SharedFileRegion_IMemory(const media::SharedFileRegion& aidl);
ConversionResult<media::SharedFileRegion>
legacy2aidl_IMemory_SharedFileRegion(const sp<IMemory>& legacy);

ConversionResult<sp<IMemory>>
aidl2legacy_NullableSharedFileRegion_IMemory(const std::optional<media::SharedFileRegion>& aidl);
ConversionResult<std::optional<media::SharedFileRegion>>
legacy2aidl_NullableIMemory_SharedFileRegion(const sp<IMemory>& legacy);

ConversionResult<AudioTimestamp>
aidl2legacy_AudioTimestamp(const media::AudioTimestampInternal& aidl);
ConversionResult<media::AudioTimestampInternal>
legacy2aidl_AudioTimestamp(const AudioTimestamp& legacy);

ConversionResult<audio_uuid_t>
aidl2legacy_AudioUuid_audio_uuid_t(const media::AudioUuid& aidl);
ConversionResult<media::AudioUuid>
legacy2aidl_audio_uuid_t_AudioUuid(const audio_uuid_t& legacy);

ConversionResult<effect_descriptor_t>
aidl2legacy_EffectDescriptor_effect_descriptor_t(const media::EffectDescriptor& aidl);
ConversionResult<media::EffectDescriptor>
legacy2aidl_effect_descriptor_t_EffectDescriptor(const effect_descriptor_t& legacy);

}  // namespace android
