#include "TestApplication.h"

#if ONAIR_OKUI_HAS_NATIVE_APPLICATION

#if !defined(ONAIR_ANDROID) && !defined(OKUI_TEST_RESOURCES_PATH)
#error You must define OKUI_TEST_RESOURCES_PATH in your build.
#endif

#ifndef ONAIR_ANDROID

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

using testing::internal::FilePath;

std::string TestApplication::ResourceManagerPath() {
#if (ONAIR_IOS || ONAIR_TVOS) && !ONAIR_SIMULATOR
    FilePath path{FilePath::ConcatPaths(FilePath(boost::filesystem::temp_directory_path().string()),
                                        FilePath("okui-tests-" ONAIR_APPLE_SDK "-bundle.xctest"))};

#else
    FilePath path{OKUI_TEST_RESOURCES_PATH};
#endif

    EXPECT_TRUE(path.FileOrDirectoryExists());
    return path.string();
}

#endif // ONAIR_ANDROID

#endif // ONAIR_OKUI_HAS_NATIVE_APPLICATION