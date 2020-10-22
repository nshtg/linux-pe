// Copyright (c) 2020 Can Boluk
// All rights reserved.   
//    
// Redistribution and use in source and binary forms, with or without   
// modification, are permitted provided that the following conditions are met: 
//    
// 1. Redistributions of source code must retain the above copyright notice,   
//    this list of conditions and the following disclaimer.   
// 2. Redistributions in binary form must reproduce the above copyright   
//    notice, this list of conditions and the following disclaimer in the   
//    documentation and/or other materials provided with the distribution.   
// 3. Neither the name of the copyright holder nor the names of its contributors
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.   
//    
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE   
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR   
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF   
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  
// POSSIBILITY OF SUCH DAMAGE.        
//
#pragma once
#include "common.hpp"
#include "dir_relocs.hpp"

#pragma pack(push, WIN_STRUCT_PACKING)
namespace win
{
    // Enclave configuration
    //
    struct enclave_config_x64_t
    {
        uint32_t                    size;
        uint32_t                    minimum_required_config_size;
        uint32_t                    policy_flags;
        uint32_t                    number_of_imports;
        uint32_t                    import_list;
        uint32_t                    import_entry_size;
        uint8_t                     family_id[ 16 ];
        uint8_t                     image_id[ 16 ];
        uint32_t                    image_version;
        uint32_t                    security_version;
        uint64_t                    enclave_size;
        uint32_t                    number_of_threads;
        uint32_t                    enclave_flags;
    };
    struct enclave_config_x86_t
    {
        uint32_t                    size;
        uint32_t                    minimum_required_config_size;
        uint32_t                    policy_flags;
        uint32_t                    number_of_imports;
        uint32_t                    import_list;
        uint32_t                    import_entry_size;
        uint8_t                     family_id[ 16 ];
        uint8_t                     image_id[ 16 ];
        uint32_t                    image_version;
        uint32_t                    security_version;
        uint32_t                    enclave_size;
        uint32_t                    number_of_threads;
        uint32_t                    enclave_flags;
    };
    template<bool x64 = default_architecture>
    using enclave_config_t = std::conditional_t<x64, enclave_config_x64_t, enclave_config_x86_t>;

    // Dynamic relocations
    //
    enum class dynamic_reloc_entry_id
    {
        guard_rf_prologue =                 1,
        guard_rf_epilogue =                 2,
        guard_import_control_transfer =     3,
        guard_indir_control_transfer =      4,
        guard_switch_table_branch =         5,
    };

    struct dynamic_reloc_guard_rf_prologue_t
    {
        uint8_t                     prologue_size;
        uint8_t                     prologue_bytes[ VAR_LEN ];
    };

    struct dynamic_reloc_guard_rf_epilogue_t
    {
        uint32_t                    epilogue_count;
        uint8_t                     epilogue_size;
        uint8_t                     branch_descriptor_element_size;
        uint16_t                    branch_descriptor_count;
        uint8_t                     branch_descriptors[ VAR_LEN ];

        inline uint8_t* get_branch_descriptor_bit_map() { return branch_descriptors + branch_descriptor_count * branch_descriptor_element_size; }
        inline const uint8_t* get_branch_descriptor_bit_map() const { return const_cast< dynamic_reloc_guard_rf_epilogue_t* >( this )->get_branch_descriptor_bit_map(); }
    };

    struct dynamic_reloc_import_control_transfer_t
    {
        uint32_t page_relative_offset       : 12;
        uint32_t indirect_call              : 1;
        uint32_t iat_index                  : 19;
    };

    struct dynamic_reloc_indir_control_transfer_t
    {
        uint16_t page_relative_offset       : 12;
        uint16_t indirect_call              : 1;
        uint16_t rex_w_prefix               : 1;
        uint16_t cfg_check                  : 1;
        uint16_t _pad0                      : 1;
    };

    struct dynamic_reloc_guard_switch_table_branch_t
    {
        uint16_t page_relative_offset       : 12;
        uint16_t register_number            : 4;
    };

    struct dynamic_reloc_x86_t
    {
        uint32_t                    symbol;
        uint32_t                    size;
        reloc_block_t               first_block;
    };

    struct dynamic_reloc_x64_t
    {
        uint64_t                    symbol;
        uint32_t                    size;
        reloc_block_t               first_block;
    };

    struct dynamic_reloc_v2_x86_t
    {
        uint32_t                    header_size;
        uint32_t                    fixup_info_size;
        uint32_t                    symbol;
        uint32_t                    symbol_group;
        uint32_t                    flags;
        uint8_t                     fixup_info[ VAR_LEN ];
    };

    struct dynamic_reloc_v2_x64_t
    {
        uint32_t                    header_size;
        uint32_t                    fixup_info_size;
        uint64_t                    symbol;
        uint32_t                    symbol_group;
        uint32_t                    flags;
        uint8_t                     fixup_info[ VAR_LEN ];
    };
    template<bool x64 = default_architecture>
    using dynamic_reloc_t =    std::conditional_t<x64, dynamic_reloc_x64_t, dynamic_reloc_x86_t>;
    template<bool x64 = default_architecture>
    using dynamic_reloc_v2_t = std::conditional_t<x64, dynamic_reloc_v2_x64_t, dynamic_reloc_v2_x86_t>;

    // Dynamic relocation table
    //
    template<bool x64 = default_architecture>
    struct dynamic_reloc_table_t
    {
        uint32_t                    version;
        uint32_t                    size;
        union
        {
            dynamic_reloc_t<x64>    first_entry; // Variable length array.
            dynamic_reloc_v2_t<x64> first_v2_entry; // Variable length array.
        };
    };
    using dynamic_reloc_table_x86_t = dynamic_reloc_table_t<false>;
    using dynamic_reloc_table_x64_t = dynamic_reloc_table_t<true>;

    // Hot patch information
    //
    struct hotpatch_base_t
    {
        uint32_t                    sequence_number;
        uint32_t                    flags;
        uint32_t                    orginal_timedate_stamp;
        uint32_t                    orginal_checksum;
        uint32_t                    code_integrity_info;
        uint32_t                    code_integrity_size;
        uint32_t                    path_table;
        uint32_t                    buffer_offset;
    };

    struct hotpatch_info_t
    {
        uint32_t                    version;
        uint32_t                    size;
        uint32_t                    sequence_number;
        uint32_t                    base_image_list;
        uint32_t                    base_image_count;
        uint32_t                    buffer_offset; 
        uint32_t                    extra_patch_size;
    };
    
    struct hotpatch_hashes_t
    {
        uint8_t                     sha256[ 32 ];
        uint8_t                     sha1[ 20 ];
    };

    // Code integrity information
    //
    struct load_config_ci_t
    {
        uint16_t                    flags;                              // Flags to indicate if CI information is available, etc.
        uint16_t                    catalog;                            // 0xFFFF means not available
        uint32_t                    rva_catalog;
        uint32_t                    _pad0;                              // Additional bitmask to be defined later
    };

    struct load_config_directory_x64_t
    {
        uint32_t                    size;
        uint32_t                    timedate_stamp;
        ex_version_t                version;
        uint32_t                    global_flags_clear;
        uint32_t                    global_flags_set;
        uint32_t                    critical_section_default_timeout;
        uint64_t                    decommit_free_block_threshold;
        uint64_t                    decommit_total_free_threshold;
        uint64_t                    lock_prefix_table;
        uint64_t                    maximum_allocation_size;
        uint64_t                    virtual_memory_threshold;
        uint64_t                    process_affinity_mask;
        uint32_t                    process_heap_flags;
        uint16_t                    csd_version;
        uint16_t                    dependent_load_flags;
        uint64_t                    edit_list;
        uint64_t                    security_cookie;
        uint64_t                    se_handler_table;
        uint64_t                    se_handler_count;
        uint64_t                    guard_cf_check_function_ptr;
        uint64_t                    guard_cf_dispatch_function_ptr;
        uint64_t                    guard_cf_function_table;
        uint64_t                    guard_cf_function_count;
        uint32_t                    guard_flags;
        load_config_ci_t            code_integrity;
        uint64_t                    guard_address_taken_iat_entry_table;
        uint64_t                    guard_address_taken_iat_entry_count;
        uint64_t                    guard_long_jump_target_table;
        uint64_t                    guard_long_jump_target_count;
        uint64_t                    dynamic_value_reloc_table;
        uint64_t                    chpe_metadata_ptr;
        uint64_t                    guard_rf_failure_routine;
        uint64_t                    guard_rf_failure_routine_function_ptr;
        uint32_t                    dynamic_value_reloc_table_offset;
        uint16_t                    dynamic_value_reloc_table_section;
        uint16_t                    _pad0;
        uint64_t                    guard_rf_verify_stack_ptr_function_ptr;
        uint32_t                    hotpatch_table_offset;
        uint32_t                    _pad1;
        uint64_t                    enclave_configuration_ptr;
        uint64_t                    volatile_metadata_ptr;
        uint64_t                    guard_eh_continuation_table;
        uint64_t                    guard_eh_continuation_table_count;
    };

    struct load_config_directory_x86_t
    {
        uint32_t                    size;
        uint32_t                    timedate_stamp;
        ex_version_t                version;
        uint32_t                    global_flags_clear;
        uint32_t                    global_flags_set;
        uint32_t                    critical_section_default_timeout;
        uint32_t                    decommit_free_block_threshold;
        uint32_t                    decommit_total_free_threshold;
        uint32_t                    lock_prefix_table;
        uint32_t                    maximum_allocation_size;
        uint32_t                    virtual_memory_threshold;
        uint32_t                    process_heap_flags;
        uint32_t                    process_affinity_mask;
        uint16_t                    csd_version;
        uint16_t                    _pad0;
        uint32_t                    edit_list;
        uint32_t                    security_cookie;
        uint32_t                    se_handler_table;
        uint32_t                    se_handler_count;
    };

    template<bool x64 = default_architecture>
    using load_config_directory_t = std::conditional_t<x64, load_config_directory_x64_t, load_config_directory_x86_t>;
};
#pragma pack(pop)