/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Exposes the same command dispatcher behind the xLights REST Automation API
// (see documentation/xlDo Commands.txt) as a Model Context Protocol server, on
// the same xFade automation port, at POST /mcp. This is the stateless flavor of
// MCP's "Streamable HTTP" transport: one JSON-RPC request in, one JSON-RPC
// response out, no Mcp-Session-Id and no server-initiated SSE stream, which is
// all HttpServer (dependencies/wxHTTPServer) can do without a much larger
// rework to support a held-open connection.

#include "../xLightsMain.h"
#include "xLightsVersion.h"

#include "nlohmann/json.hpp"

#include "AutomationJson.h"
#include "../../dependencies/wxHTTPServer/wxhttpserver.h"

#include <log.h>
#include <map>
#include <string>
#include <vector>

namespace {
    const char* MCP_PROTOCOL_VERSION = "2025-06-18";
    const wxString MIME_JSON = "application/json";

    struct MCPToolDef {
        std::string name;
        std::string description;
        std::string automationCmd; // empty => generic passthrough (xlights_command)
        nlohmann::json inputSchema;
    };

    nlohmann::json Schema(const char* json) {
        return nlohmann::json::parse(json);
    }

    const std::vector<MCPToolDef>& GetMCPTools() {
        static const std::vector<MCPToolDef> tools = {
            { "xlights_command",
              "Call any xLights automation command by name. Use this for anything not covered by a "
              "dedicated tool -- the full command list and their parameters are documented in "
              "xLights' 'documentation/xlDo Commands.txt'.",
              "",
              Schema(R"({
                "type": "object",
                "properties": {
                    "cmd": { "type": "string", "description": "Automation command name, e.g. 'renderAll', 'uploadController', 'batchRender'." },
                    "params": { "type": "object", "description": "Command-specific parameters as key/value pairs. An array value is flattened to name_0, name_1, ... automatically, matching the xlDoAutomation JSON protocol." }
                },
                "required": ["cmd"]
              })") },
            { "getVersion", "Get the running xLights version.", "getVersion",
              Schema(R"({"type":"object","properties":{}})") },
            { "getShowFolder", "Get the current xLights show folder.", "getShowFolder",
              Schema(R"({"type":"object","properties":{}})") },
            { "getOpenSequence", "Get details of the currently open sequence, if any.", "getOpenSequence",
              Schema(R"({"type":"object","properties":{}})") },
            { "openSequence", "Open a sequence file in xLights.", "openSequence",
              Schema(R"({
                "type": "object",
                "properties": {
                    "seq": { "type": "string", "description": "Sequence filename or path to open." },
                    "force": { "type": "boolean", "description": "Force opening even if a sequence is already open." },
                    "promptIssues": { "type": "boolean", "description": "Prompt for any issues found while opening." }
                },
                "required": ["seq"]
              })") },
            { "closeSequence", "Close the currently open sequence.", "closeSequence",
              Schema(R"({
                "type": "object",
                "properties": {
                    "quiet": { "type": "boolean", "description": "Return success even if no sequence was open." },
                    "force": { "type": "boolean", "description": "Close even if there are unsaved changes." }
                }
              })") },
            { "saveSequence", "Save the currently open sequence.", "saveSequence",
              Schema(R"({
                "type": "object",
                "properties": {
                    "seq": { "type": "string", "description": "Filename to save as; omit to save using the sequence's current name." }
                }
              })") },
            { "renderAll", "Render the currently open sequence.", "renderAll",
              Schema(R"({
                "type": "object",
                "properties": {
                    "highdef": { "type": "boolean", "description": "Render at high definition." }
                }
              })") },
            { "getModels", "Get the list of models and/or model groups defined in the layout.", "getModels",
              Schema(R"({
                "type": "object",
                "properties": {
                    "models": { "type": "boolean", "description": "Include models (default true)." },
                    "groups": { "type": "boolean", "description": "Include model groups (default true)." }
                }
              })") },
            { "getControllers", "Get the list of configured controllers.", "getControllers",
              Schema(R"({"type":"object","properties":{}})") },
            { "getEffectSettings", "Get the settings, palette, and timing of one effect on a model.", "getEffectSettings",
              Schema(R"({
                "type": "object",
                "properties": {
                    "model": { "type": "string", "description": "Model name." },
                    "layer": { "type": "string", "description": "Effect layer index, as a string." },
                    "id": { "type": "string", "description": "Effect id within the layer, as a string." }
                },
                "required": ["model", "layer", "id"]
              })") },
            { "setEffectSettings", "Set the settings, palette, and/or timing of one effect on a model.", "setEffectSettings",
              Schema(R"({
                "type": "object",
                "properties": {
                    "model": { "type": "string", "description": "Model name." },
                    "layer": { "type": "string", "description": "Effect layer index, as a string." },
                    "id": { "type": "string", "description": "Effect id within the layer, as a string." },
                    "name": { "type": "string", "description": "Effect type name, if changing it." },
                    "settings": { "type": "object", "description": "Effect settings map, e.g. {\"E_TEXTCTRL_Eff_On_Start\":\"10\"}." },
                    "palette": { "type": "object", "description": "Color palette settings map, e.g. {\"C_BUTTON_Palette1\":\"#0000FF\"}." },
                    "startTime": { "type": "string", "description": "Start time in ms, as a string." },
                    "endTime": { "type": "string", "description": "End time in ms, as a string." }
                },
                "required": ["model", "layer", "id"]
              })") },
            { "addEffect", "Add a new effect to a model.", "addEffect",
              Schema(R"({
                "type": "object",
                "properties": {
                    "target": { "type": "string", "description": "Model to add the effect to." },
                    "effect": { "type": "string", "description": "Effect type name, e.g. 'On', 'Bars', 'Butterfly'." },
                    "settings": { "type": "object", "description": "Effect settings map." },
                    "palette": { "type": "object", "description": "Color palette settings map." },
                    "layer": { "type": "string", "description": "Effect layer to add to (default 0)." },
                    "startTime": { "type": "string", "description": "Start time in ms." },
                    "endTime": { "type": "string", "description": "End time in ms." }
                },
                "required": ["target", "effect", "startTime", "endTime"]
              })") },
            { "deleteEffect", "Delete one effect from a model.", "deleteEffect",
              Schema(R"({
                "type": "object",
                "properties": {
                    "model": { "type": "string", "description": "Model name." },
                    "layer": { "type": "string", "description": "Effect layer index, as a string." },
                    "id": { "type": "string", "description": "Effect id within the layer, as a string." }
                },
                "required": ["model", "layer", "id"]
              })") },
            { "lightsOn", "Turn output to lights on.", "lightsOn",
              Schema(R"({"type":"object","properties":{}})") },
            { "lightsOff", "Turn output to lights off.", "lightsOff",
              Schema(R"({"type":"object","properties":{}})") },
        };
        return tools;
    }

    const MCPToolDef* FindMCPTool(const std::string& name) {
        for (auto& t : GetMCPTools()) {
            if (t.name == name) return &t;
        }
        return nullptr;
    }

    // Built with explicit object()/array() + assignment throughout, rather than brace-init
    // json literals: nlohmann decides object-vs-array per nesting level by inspecting the
    // already-constructed element values (a list is an object only if every element resolved
    // to a 2-item array with a string first item), so a brace literal can silently resolve to
    // the wrong shape with no compile error. Explicit construction has no such ambiguity.
    nlohmann::json MakeToolListJson() {
        nlohmann::json arr = nlohmann::json::array();
        for (auto& t : GetMCPTools()) {
            nlohmann::json tj = nlohmann::json::object();
            tj["name"] = t.name;
            tj["description"] = t.description;
            tj["inputSchema"] = t.inputSchema;
            arr.push_back(tj);
        }
        nlohmann::json result = nlohmann::json::object();
        result["tools"] = arr;
        return result;
    }

    nlohmann::json MakeToolResultJson(const std::string& text, bool isError) {
        nlohmann::json contentItem = nlohmann::json::object();
        contentItem["type"] = "text";
        contentItem["text"] = text;
        nlohmann::json arr = nlohmann::json::array();
        arr.push_back(contentItem);
        nlohmann::json result = nlohmann::json::object();
        result["content"] = arr;
        result["isError"] = isError;
        return result;
    }

    nlohmann::json MakeRpcResult(const nlohmann::json& id, const nlohmann::json& result) {
        nlohmann::json resp = nlohmann::json::object();
        resp["jsonrpc"] = "2.0";
        resp["id"] = id;
        resp["result"] = result;
        return resp;
    }

    nlohmann::json MakeRpcError(const nlohmann::json& id, int code, const std::string& message) {
        nlohmann::json err = nlohmann::json::object();
        err["code"] = code;
        err["message"] = message;
        nlohmann::json resp = nlohmann::json::object();
        resp["jsonrpc"] = "2.0";
        resp["id"] = id;
        resp["error"] = err;
        return resp;
    }
}

bool xLightsFrame::ProcessMCPRequest(HttpConnection& connection, HttpRequest& request)
{
    auto send = [&](HttpStatus::HttpStatusCode code, const wxString& mime, const std::string& body) {
        HttpResponse resp(connection, request, code);
        resp.AddHeader("access-control-allow-origin", "*");
        resp.MakeFromText(body, mime);
        if (_automationServer != nullptr && _automationServer->IsConnectionValid(&connection)) {
            connection.SendResponse(resp);
            return true;
        }
        spdlog::warn("MCP did not send result because connection lost.");
        return false;
    };

    if (request.Method() != "POST") {
        return send(HttpStatus::OK, "text/plain",
                     "xLights MCP server. POST JSON-RPC 2.0 requests here (Streamable HTTP transport, stateless).");
    }

    nlohmann::json req;
    try {
        req = nlohmann::json::parse(request.Data().ToStdString());
    } catch (const std::exception&) {
        // Use HttpStatus::OK, not BadRequest: a 400 here makes the underlying
        // HTTP server auto-inject its own configured 400 error page (see
        // ctx.ErrorPage400 below) on top of the JSON-RPC error body we just
        // sent, corrupting the response. JSON-RPC-over-HTTP convention is to
        // report the error inside the envelope with a 200 anyway, matching
        // every other error path in this file.
        return send(HttpStatus::OK, MIME_JSON, MakeRpcError(nullptr, -32700, "Parse error.").dump());
    }
    if (!req.is_object() || !req.contains("method") || !req["method"].is_string()) {
        return send(HttpStatus::OK, MIME_JSON, MakeRpcError(nullptr, -32600, "Invalid request.").dump());
    }

    std::string method = req["method"].get<std::string>();
    nlohmann::json params = (req.contains("params") && req["params"].is_object()) ? req["params"] : nlohmann::json::object();
    bool isNotification = !req.contains("id");
    nlohmann::json id = isNotification ? nlohmann::json(nullptr) : req["id"];

    if (isNotification) {
        // Nothing to do for notifications/initialized, notifications/cancelled, etc. --
        // this server keeps no per-session state to update.
        return send(HttpStatus::Accepted, "text/plain", "");
    }

    nlohmann::json response;
    if (method == "initialize") {
        nlohmann::json capabilities = nlohmann::json::object();
        capabilities["tools"] = nlohmann::json::object();
        nlohmann::json serverInfo = nlohmann::json::object();
        serverInfo["name"] = "xlights-automation";
        serverInfo["version"] = GetDisplayVersionString();
        nlohmann::json initResult = nlohmann::json::object();
        initResult["protocolVersion"] = MCP_PROTOCOL_VERSION;
        initResult["capabilities"] = capabilities;
        initResult["serverInfo"] = serverInfo;
        response = MakeRpcResult(id, initResult);
    } else if (method == "ping") {
        response = MakeRpcResult(id, nlohmann::json::object());
    } else if (method == "tools/list") {
        response = MakeRpcResult(id, MakeToolListJson());
    } else if (method == "tools/call") {
        std::string toolName = params.value("name", "");
        nlohmann::json args = (params.contains("arguments") && params["arguments"].is_object()) ? params["arguments"] : nlohmann::json::object();

        std::vector<std::string> autoPaths;
        std::map<std::string, std::string> autoParams;

        if (toolName == "xlights_command") {
            if (!args.contains("cmd") || !args["cmd"].is_string()) {
                response = MakeRpcError(id, -32602, "Missing required 'cmd' argument.");
                return send(HttpStatus::OK, MIME_JSON, response.dump());
            }
            autoPaths.push_back(args["cmd"].get<std::string>());
            if (args.contains("params") && args["params"].is_object()) {
                FlattenAutomationParams(args["params"], autoParams);
            }
        } else {
            const MCPToolDef* tool = FindMCPTool(toolName);
            if (tool == nullptr) {
                response = MakeRpcError(id, -32602, "Unknown tool: '" + toolName + "'.");
                return send(HttpStatus::OK, MIME_JSON, response.dump());
            }
            autoPaths.push_back(tool->automationCmd);
            FlattenAutomationParams(args, autoParams);
        }
        autoParams["_METHOD"] = "POST";

        std::string msgOut, jsonKeyOut;
        int codeOut = 200;
        bool isJsonOut = false;
        bool processed = ProcessAutomation(autoPaths, autoParams, [&](const std::string& msg, const std::string& jsonKey, int responseCode, bool isJson) {
            msgOut = msg;
            jsonKeyOut = jsonKey;
            codeOut = responseCode;
            isJsonOut = isJson;
            return true;
        });

        std::string bodyText = processed ? BuildAutomationResponseJson(codeOut, jsonKeyOut, msgOut, isJsonOut)
                                          : ("{\"res\":504,\"msg\":\"Unknown command: '" + autoPaths[0] + "'.\"}");
        response = MakeRpcResult(id, MakeToolResultJson(bodyText, !processed || codeOut >= 400));
    } else {
        response = MakeRpcError(id, -32601, "Method not found: '" + method + "'.");
    }

    return send(HttpStatus::OK, MIME_JSON, response.dump());
}
