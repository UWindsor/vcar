#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <restbed>
#include <string>
#include <vcar.h>

#define VID_JSON_FILE "vids.json"

void PostRegisterUser(std::shared_ptr<restbed::Session> session);
void PostVehicle(std::shared_ptr<restbed::Session> session);
void GetVehicle(std::shared_ptr<restbed::Session> session);
void UpdateVehicle(std::shared_ptr<restbed::Session> session);
void DeleteVehicle(std::shared_ptr<restbed::Session> session);

// JSON helper functions
rapidjson::Value vcarToJSON(std::string vid);
std::string vcarToJSONString(std::string vid);
void cleanJSONString(std::string& filthy_json);
std::string getStringFromJSON(rapidjson::Value const& doc);
void parseJSONFromFile(std::string file_name, rapidjson::Document& doc);
void saveJSONToFile(std::string file_name, rapidjson::Value& doc);

rapidjson::Document vids;
bool running = true;
bool default_halted = true;
std::map<std::string, vcar> vehicles {};

int main() {
    // Load vid files JSON
    parseJSONFromFile(VID_JSON_FILE, vids);

    std::cout << "Loaded from file: " << getStringFromJSON(vids) << std::endl;

    /// REST communication setup
    auto register_user = std::make_shared<restbed::Resource>();
    auto register_vehicle = std::make_shared<restbed::Resource>();
    auto vehicle = std::make_shared<restbed::Resource>();

    register_user->set_path("/embedded/v1/register");
    register_vehicle->set_path("/embedded/v1/vehicles");
    vehicle->set_path("/embedded/v1/vehicles/{vid: ([^\\s]+)}");

    register_user->set_method_handler("POST", PostRegisterUser);
    register_vehicle->set_method_handler("POST", PostVehicle);
    vehicle->set_method_handler("GET", GetVehicle);
    vehicle->set_method_handler("UPDATE", UpdateVehicle);
    vehicle->set_method_handler("DELETE", DeleteVehicle);

    auto settings = std::make_shared<restbed::Settings>();
    settings->set_port(7998);
    settings->set_default_header("Connection", "close");

    restbed::Service service;
    service.publish(register_vehicle);
    service.publish(register_user);
    service.publish(vehicle);
    service.start(settings);

    while (running) {}

    return 0;
}

void PostRegisterUser(const std::shared_ptr<restbed::Session> session) {
    using namespace rapidjson;

    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        std::stringstream user_ss;
        user_ss << body.data() << std::flush;

        Document user_json;
        user_json.Parse(user_ss.str().c_str());

        if (user_json.HasParseError()) {
             throw std::runtime_error("Error parsing user JSON");
        }

        std::cout << (int)body.size() << " | " << body.data() << std::endl;
        session->close(restbed::OK, "POST Register User");
    });
}

void PostVehicle(std::shared_ptr<restbed::Session> session) {
    using namespace rapidjson;

    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        // Clean incoming data JSON
        std::stringstream vehicle_ss;
        vehicle_ss << body.data() << std::flush;
        std::string vjson_str = vehicle_ss.str();
        cleanJSONString(vjson_str);

        // Parse data JSON
        Document vehicle_json;
        vehicle_json.Parse(vjson_str.c_str());
        std::cout << "POST vehicle: " << vjson_str.c_str() << std::endl;

        if (vehicle_json.HasParseError()) {
            throw std::runtime_error("Error parsing vehicle JSON");
        }

        std::string email = vehicle_json["email"].GetString();
        std::string vid = vehicle_json["vid"].GetString();
        std::string vtype = vehicle_json["vtype"].GetString();

        // Vehicle already exists
        if (vehicles.find(vid) != vehicles.end()) {
            session->close(restbed::CONFLICT);
            return;
        }

        // New email
        if (!vids.HasMember(email.c_str())) {
            Value vid_list(kArrayType);
            vids.AddMember(Value(email.c_str(), vids.GetAllocator()), vid_list, vids.GetAllocator());
        }

        // Vehicle limit
        if (vids[email.c_str()].GetArray().Size() >= 5) {
            session->close(restbed::BAD_REQUEST, "Vehicle limit reached");
            return;
        }


        // Create car
        vehicles.emplace(std::piecewise_construct, std::forward_as_tuple(vid), std::forward_as_tuple(default_halted, vtype));

        // Create vid entry
        vids[email.c_str()].GetArray().PushBack(vcarToJSON(vid), vids.GetAllocator());

        saveJSONToFile(VID_JSON_FILE, vids);

        session->close( restbed::CREATED, "New vehicle created");
    });
}

void GetVehicle(const std::shared_ptr<restbed::Session> session) {
    using namespace rapidjson;

    const auto req = session->get_request();

    std::string vid = req->get_path_parameter("vid");

    std::cout << "In vids: " << getStringFromJSON(vids) << std::endl;
    for (auto vit = vehicles.begin(); vit != vehicles.end(); vit++) {
        std::cout << vit->first << " ";
    }
    std::cout << "|" << std::endl;

    if (vehicles.find(vid) == vehicles.end()) {
        session->close(restbed::NOT_FOUND);
        return;
    }

    session->close( restbed::OK, vcarToJSONString(vid), {{"Content-Type", "application/json"}});
}

void UpdateVehicle(const std::shared_ptr<restbed::Session> session) {
    using namespace rapidjson;

    const auto req = session->get_request();
    int content_length = req->get_header("Content-Length", 0);

    session->fetch(content_length, [](const std::shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        std::stringstream vehicle_ss;
        vehicle_ss << body.data() << std::flush;

        Document vehicle_json;
        vehicle_json.Parse(vehicle_ss.str().c_str());

        if (vehicle_json.HasParseError()) {
            throw std::runtime_error("Error parsing user JSON");
        }

        session->close( restbed::OK, "UPDATE Vehicle", { { "Content-Length", "13" } } );
    });
}

void DeleteVehicle(const std::shared_ptr<restbed::Session> session) {
    using namespace rapidjson;

    const auto req = session->get_request();

    std::string vid = req->get_path_parameter("vid");

    auto vit = vehicles.find(vid);
    if (vit == vehicles.end()) {
        session->close(restbed::NOT_FOUND);
        return;
    }

    std::string vehicle_to_erase = vcarToJSONString(vid);

    for (auto user_it = vids.MemberBegin(); user_it != vids.MemberEnd(); user_it++) {
        Value user_array = user_it->value.GetArray();

        for (size_t i = 0; i < user_array.Size(); i++) {
            Value vehicle_object = user_array[i].GetObject();
            if (vehicle_object["vid"].GetString() == vid) {
                user_array.Erase(user_array.Begin() + i);
            }
        }
    }

    saveJSONToFile(VID_JSON_FILE, vids);
    vehicles.erase(vit);

    session->close( restbed::OK, vehicle_to_erase, {{"Content-Type", "application/json"}});
}

/// JSON helper functions definitions

rapidjson::Value vcarToJSON(std::string vid) {
    using namespace rapidjson;

    Value car_json(kObjectType);

    car_json.AddMember("vid", Value(vid.c_str(), vids.GetAllocator()), vids.GetAllocator());
    car_json.AddMember("type", Value(vehicles[vid].getType().c_str(), vids.GetAllocator()), vids.GetAllocator());

    return car_json;
}

std::string vcarToJSONString(std::string vid) {
    using namespace rapidjson;

    assert(vehicles.find(vid) != vehicles.end());

    Value car_json(kObjectType);

    car_json.AddMember("vid", Value(vid.c_str(), vids.GetAllocator()), vids.GetAllocator());
    car_json.AddMember("type", Value(vehicles[vid].getType().c_str(), vids.GetAllocator()), vids.GetAllocator());

    return getStringFromJSON(car_json);
}

void cleanJSONString(std::string& filthy_json) {
    filthy_json.erase(std::remove(filthy_json.begin(), filthy_json.end(), '\\'), filthy_json.end());  // Erase slashes

    // Erase surrounding quotes
    if (*filthy_json.begin() == '\"' && *(filthy_json.end() - 1) == '\"') {
        filthy_json = filthy_json.substr(1, filthy_json.size() - 2);
    }
}

std::string getStringFromJSON(rapidjson::Value const& doc) {
    rapidjson::StringBuffer buffer;
    buffer.Clear();

    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::string ret = std::string(strdup(buffer.GetString()));
    cleanJSONString(ret);

    return ret;
}

void parseJSONFromFile(const std::string file_name, rapidjson::Document& doc) {
    using namespace rapidjson;

    // Load vid file
    std::ifstream fs(file_name);
    fs.seekg(0, fs.end);
    int file_length = fs.tellg();
    fs.seekg(0, fs.beg);

    char* file_content = new char[file_length];
    fs.read(file_content, file_length);

    doc.Parse(file_content);

    if (doc.HasParseError()) {
        throw std::runtime_error("Error parsing user JSON");
    }
    assert(doc.IsObject());

    for (auto user_it = doc.GetObject().MemberBegin(); user_it != doc.GetObject().MemberEnd(); user_it++) {
        std::string user_email = user_it->name.GetString();
        for (int i = 0; i < doc[user_email.c_str()].Size(); i++) {
            vehicles.emplace(std::piecewise_construct, std::forward_as_tuple(doc[user_email.c_str()][i]["vid"].GetString()), std::forward_as_tuple(default_halted, doc[user_email.c_str()][i]["type"].GetString()));
        }
    }

    delete[] file_content;
}

void saveJSONToFile(const std::string file_name, rapidjson::Value& doc) {
    std::ofstream fs(file_name, std::iostream::trunc);
    fs << getStringFromJSON(doc);
    fs.close();
}
