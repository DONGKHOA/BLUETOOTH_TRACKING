# Include filepaths for source and include.
include( ${MODULE_ROOT_DIR}/httpFilePaths.cmake )

# ====================  Define your project name (edit) ========================
set(project_name "core_http")

# =====================  Create your mock here  (edit)  ========================

# list the files to mock here
list(APPEND mock_list
            ${LLHTTP_DIR}/include/llhttp.h
        )
# list the directories your mocks need
list(APPEND mock_include_list
            ${LLHTTP_DIR}/include
        )
#list the definitions of your mocks to control what to be included
list(APPEND mock_define_list
            ""
       )

# ================= Create the library under test here (edit) ==================

# list the files you would like to test here
list(APPEND real_source_files
            ${HTTP_SOURCES}
        )
# list the directories the module under test includes
list(APPEND real_include_directories
            .
            ${HTTP_INCLUDE_PUBLIC_DIRS}
        )

# =====================  Create UnitTest Code here (edit)  =====================

# list the directories your test needs to include
list(APPEND test_include_directories
            .
            ${HTTP_INCLUDE_PUBLIC_DIRS}
        )

# =============================  (end edit)  ===================================

set(mock_name "llhttp_mock")
set(real_name "${project_name}_real")

create_mock_list(${mock_name}
                "${mock_list}"
                "${MODULE_ROOT_DIR}/tools/cmock/project.yml"
                "${mock_include_list}"
                "${mock_define_list}"
        )

create_real_library(${real_name}
                    "${real_source_files}"
                    "${real_include_directories}"
                    "${mock_name}"
        )

list(APPEND utest_link_list
            -l${mock_name}
            lib${real_name}.a
        )

list(APPEND utest_dep_list
            ${real_name}
        )

set(utest_name "${project_name}_utest")
set(utest_source "${project_name}_utest.c")
create_test(${utest_name}
            ${utest_source}
            "${utest_link_list}"
            "${utest_dep_list}"
            "${test_include_directories}"
        )

set(utest_name "${project_name}_send_utest")
set(utest_source "${project_name}_send_utest.c")
create_test(${utest_name}
            ${utest_source}
            "${utest_link_list}"
            "${utest_dep_list}"
            "${test_include_directories}"
        )
