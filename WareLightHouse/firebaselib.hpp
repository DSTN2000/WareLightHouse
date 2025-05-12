#ifndef FIREBASELIB_HPP
#define FIREBASELIB_HPP
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <regex>
#include <QTextDocument>
#include <QString>
#include <QUrl>

using json = nlohmann::json;

class FirebaseDB {
private:
    std::string database_url;

public:
    FirebaseDB(const std::string& url) : database_url(url) {
        // Ensure URL ends with /
        if (database_url.back() != '/') {
            database_url += '/';
        }
    }

    // Read data from a specific path
    json readData(const std::string& path) {
        // Construct the URL for the specific path with .json suffix
        std::string url = database_url + path + ".json";

        // Make GET request
        cpr::Response r = cpr::Get(cpr::Url{url});

        // Check for errors
        if (r.status_code != 200) {
            std::cerr << "Error reading data: " << r.text << std::endl;
            return json::object();
        }

        // Return parsed JSON data
        if (!json::parse(r.text).empty())
        {
            return json::parse(r.text);
        }
        else
        {
            return {};
        }
    }

    // Write data to a specific path
    bool writeData(const std::string& path, const json& data) {
        // Construct the URL for the specific path with .json suffix
        std::string url = database_url + path + ".json";

        // Make PUT request with JSON data
        cpr::Response r = cpr::Put(
            cpr::Url{url},
            cpr::Header{{"Content-Type", "application/json"}},
            cpr::Body{data.dump()}
            );

        // Check for errors
        if (r.status_code != 200) {
            std::cerr << "Error writing data: " << r.text << std::endl;
            return false;
        }

        std::cout << "Data written successfully!" << std::endl;
        return true;
    }

    // Update specific fields at a path
    bool updateData(const std::string& path, const json& data) {
        // Construct the URL for the specific path with .json suffix
        std::string url = database_url + path + ".json";

        // Make PATCH request with JSON data
        cpr::Response r = cpr::Patch(
            cpr::Url{url},
            cpr::Header{{"Content-Type", "application/json"}},
            cpr::Body{data.dump()}
            );

        // Check for errors
        if (r.status_code != 200) {
            std::cerr << "Error updating data: " << r.text << std::endl;
            return false;
        }

        std::cout << "Data updated successfully!" << std::endl;
        return true;
    }

    // Delete data at a specific path
    bool deleteData(const std::string& path) {
        // Construct the URL for the specific path with .json suffix
        std::string url = database_url + path + ".json";

        // Make DELETE request
        cpr::Response r = cpr::Delete(cpr::Url{url});

        // Check for errors
        if (r.status_code != 200) {
            std::cerr << "Error deleting data: " << r.text << std::endl;
            return false;
        }

        std::cout << "Data deleted successfully!" << std::endl;
        return true;
    }

    // Add new data with auto-generated key (push operation)
    std::string pushData(const std::string& path, const json& data) {
        // Construct the URL for the specific path with .json suffix
        std::string url = database_url + path + ".json";

        // Make POST request with JSON data
        cpr::Response r = cpr::Post(
            cpr::Url{url},
            cpr::Header{{"Content-Type", "application/json"}},
            cpr::Body{data.dump()}
            );

        // Check for errors
        if (r.status_code != 200) {
            std::cerr << "Error pushing data: " << r.text << std::endl;
            return "";
        }

        // Parse response to get the auto-generated key
        json response = json::parse(r.text);
        std::string new_key = response["name"];

        std::cout << "Data pushed successfully with key: " << new_key << std::endl;
        return new_key;
    }

    // #------------------------ PROJECT-SPECIFIC FUNCTIONS ------------------------#
    bool addUser(std::string company, std::string username, std::string password)
    {

        json companies = this->readData("companies");
        //std::cout << companies.items();
        for (auto &pair: companies.items())
        {
            //std::cout<<pair.key()<<'\n';
            if (company==pair.key())
            {
                return false;
            }
        }
        company = urlEncode(company);
        username = urlEncode(username);
        json data = {
            {"password",password}
        };
        this->writeData("companies/"+company+"/users/"+username, data);
        return true;
    }

    bool authenticateUser(std::string company, std::string username, std::string password)
    {
        company = urlEncode(company);
        username = urlEncode(username);
        json userPassword = this->readData("companies/"+company+"/users/"+username+"/");
        //std::cout << userPassword;
        if (userPassword.empty()||userPassword["password"]!=password) return false;
        return true;
    }

    std::string urlEncode(std::string str)
    {
        return QUrl::toPercentEncoding(QString::fromStdString(str)).toStdString();
    }

};

#endif // FIREBASELIB_HPP
