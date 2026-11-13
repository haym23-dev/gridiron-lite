# Shared compiler and linker options for every Gridiron target.
#
# Call gridiron_apply_build_flags(<target>) once per target.  Flags are applied
# per target rather than globally so that third_party/ libraries can keep their
# own defaults (many of them rely on exceptions and RTTI).

include(CheckIPOSupported)

# LTO support is a property of the toolchain, so probe it once here.
check_ipo_supported(RESULT GRIDIRON_IPO_SUPPORTED OUTPUT GRIDIRON_IPO_ERROR)

function(gridiron_apply_build_flags target)
  target_compile_features(${target} PRIVATE cxx_std_20)

  if(MSVC)
    target_compile_options(${target} PRIVATE
      /GR-                # no RTTI
      /EHs-c-             # no exceptions
      $<$<CONFIG:Debug>:/Od;/Zi;/RTC1>
      $<$<CONFIG:Release>:/O2;/Gy;/GS->
      $<$<CONFIG:Profile>:/O2;/Zi>
    )
    target_link_options(${target} PRIVATE
      $<$<CONFIG:Release>:/OPT:REF;/OPT:ICF>
    )
  else()
    target_compile_options(${target} PRIVATE
      -fno-exceptions
      -fno-rtti
      $<$<CONFIG:Debug>:-O0;-g3>
      $<$<CONFIG:Release>:-O2;-ffunction-sections;-fdata-sections;-fvisibility=hidden>
      $<$<CONFIG:Profile>:-O2;-g;-fno-omit-frame-pointer>
    )

    if(APPLE)
      # ld64 uses -dead_strip instead of --gc-sections and has no bare -s.
      target_link_options(${target} PRIVATE
        $<$<CONFIG:Release>:-Wl,-dead_strip>
      )
    else()
      target_link_options(${target} PRIVATE
        $<$<CONFIG:Release>:-Wl,--gc-sections;-s>
      )
    endif()
  endif()

  # Link-time optimisation belongs to Release only; Profile keeps functions
  # intact so profilers can still name them.
  if(GRIDIRON_IPO_SUPPORTED)
    set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(WARNING "LTO requested for Release but unsupported: ${GRIDIRON_IPO_ERROR}")
  endif()
endfunction()
