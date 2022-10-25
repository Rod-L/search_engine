#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>

namespace PathHelper {
    /**
     * @return current working directory of the process
     */
    std::string current_dir();

    /**
     * @param filepath path to file or catalog
     * @return true, if passed filepath is relative
     */
    bool is_relative(const std::string& filepath);

    /**
     * Combines root path and relative path
     * @param root path to file or catalog
     * @param relative relative path to file or catalog from root path
     * @return result of path combination
     *
     * example:
     * root     = "C:/catalog"
     * relative = "../file"
     * result   = C:/file
     */
    std::string combine_path(const std::string& root, const std::string& relative);

    /**
     * @param filepath path to file
     * @return path to catalog
     *
     * example:
     * filepath = C:/catalog/file
     * result   = C:/catalog/
     */
    std::string catalog_from_filepath(const std::string& filepath);

    /**
     * @param filepath path to file
     * @return name of the file without extension
     */
    std::string file_name(const std::string& filepath);

    /**
     * @param filepath path to file
     * @return extension of the file
     */
    std::string file_extension(const std::string& filepath);

    /**
     * @param filepath path to file
     * @return true, if file by passed filepath exist
     */
    bool file_exist(const std::string& filepath);

    /**
     * @param path path to the file
     * @param acceptor string stream, where content of the file should be placed
     * @return true, if content was fetched successully
     */
    bool get_text(const std::string& path, std::stringstream& acceptor);
}