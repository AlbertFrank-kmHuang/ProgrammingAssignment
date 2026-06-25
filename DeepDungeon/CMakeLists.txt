cmake_minimum_required(VERSION 3.16)
project(DeepDungeon VERSION 0.1 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用Qt的自动处理
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)  # 确保MOC自动运行
set(CMAKE_AUTORCC OFF)  # 手动处理资源

# 查找Qt6组件
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Multimedia
)

# 源文件列表
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    prepare.h prepare.cpp prepare.ui
    review.h review.cpp review.ui
    shop.h shop.cpp shop.ui
    battle.h battle.cpp battle.ui
    character_config.h
    character.cpp
    enemy_config.h enemy.cpp
    weapon_config.h weapon.cpp
    card_config.h card.cpp
    event.h event.cpp event.ui event_config.h
    benchlabel.cpp benchlabel.h
    fighterlabel.cpp fighterlabel.h
    battle_engine.h battle_engine.cpp
    # 新增以下两行
    clickablelabel.h
    clickablelabel.cpp
    # 音效 / 特效 / 主题 / 图鉴 / 新手引导
    soundmanager.h soundmanager.cpp
    effects.h effects.cpp
    gametheme.h
    codex.h codex.cpp
    tutorial.h tutorial.cpp
    app_icon.rc  # <-- 新增：您手动创建的Windows资源文件
)

# 资源文件列表
set(QRC_FILES
    resources.qrc
    backgrounds.qrc
)

# 手动运行 rcc 生成资源文件
foreach(qrc_file ${QRC_FILES})
    get_filename_component(qrc_name ${qrc_file} NAME_WE)
    set(rcc_output ${CMAKE_CURRENT_BINARY_DIR}/qrc_${qrc_name}.cpp)

    # 添加自定义命令运行 rcc
    add_custom_command(
        OUTPUT ${rcc_output}
        COMMAND Qt6::rcc
        ARGS -name ${qrc_name} ${CMAKE_CURRENT_SOURCE_DIR}/${qrc_file} -o ${rcc_output}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${qrc_file}
        VERBATIM
    )

    # 将生成的资源文件添加到源文件列表
    list(APPEND GENERATED_SOURCES ${rcc_output})
endforeach()

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${GENERATED_SOURCES}  # 使用手动生成的资源文件
)

# 设置输出目录（可选，便于管理）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# 包含当前目录
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Multimedia
)

# 设置可执行文件的属性
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# 安装目标（可选）
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)

# 如果Qt版本是6.0.0或更高，添加最终化步骤
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.0.0)
    qt_standard_project_setup()
    qt_finalize_executable(${PROJECT_NAME})
endif()