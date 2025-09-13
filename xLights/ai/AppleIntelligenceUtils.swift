//
//  AppleIntelligenceUtils.swift
//  xLights-macOSLib
//
//  Created by Daniel Kulp on 9/13/25.
//  Copyright © 2025 Daniel Kulp. All rights reserved.
//

import Foundation
import FoundationModels

@available(macOS 26.0, *)
struct DynObjCreator {
    let name: String
    var properties: [DynamicGenerationSchema.Property] = []
    
    mutating func addStringProperty(name: String) {
        let property = DynamicGenerationSchema.Property(
            name: name,
            schema: DynamicGenerationSchema(type: String.self)
        )
        properties.append(property)
    }
    mutating func addArrayProperty(name: String, customType: String) {
        let property = DynamicGenerationSchema.Property(
            name: name,
            schema: DynamicGenerationSchema (
                arrayOf: DynamicGenerationSchema(referenceTo: customType)
            )
        )
        properties.append(property)
    }
    var root: DynamicGenerationSchema {
        DynamicGenerationSchema(
          name: name,
          properties: properties
        )
    }
}


class AsyncStringResult: @unchecked Sendable {
    var result: String = ""
}

public func RunAppleIntelligencePrompt(_ prompt: String) -> String {
    if #available(macOS 26.0, *) {
        let semaphore = DispatchSemaphore(value: 0)
        let result: AsyncStringResult = .init()
        Task {
            defer {
                semaphore.signal()
            }
            do {
                let session = LanguageModelSession()
                let response = try await session.respond(to: prompt)
                result.result = response.content
            } catch {
                // You may want to log this error or surface it to callers in the future
                print("exception!  \(error)\n");
            }
        }
        semaphore.wait()
        return result.result
    } else {
        return ""
    }
}

public func RunAppleIntelligenceGeneratePalette(_ prompt: String) -> String {
    let fullprompt = "xlights color paletes are 8 unique colors. Can you create a color palette that would represent the moods and imagery " + prompt + ". Avoid dark, near black colors."
    if #available(macOS 26.0, *) {
        let semaphore = DispatchSemaphore(value: 0)
        let result: AsyncStringResult = .init()
        Task {
            defer {
                semaphore.signal()
            }
            var colorPaletteBuilder = DynObjCreator(name: "ColorPalette")
            var color = DynObjCreator(name: "Color")
            color.addStringProperty(name: "Name")
            color.addStringProperty(name: "Hex Value")
            color.addStringProperty(name: "Description")
            colorPaletteBuilder.addArrayProperty(name: "Colors", customType: "Color")
            colorPaletteBuilder.addStringProperty(name: "Song")
            colorPaletteBuilder.addStringProperty(name: "Artist")
            colorPaletteBuilder.addStringProperty(name: "Description")
            
            let colorPaletteDynamicSchema = colorPaletteBuilder.root
            let schema = try GenerationSchema(
              root: colorPaletteDynamicSchema,
              dependencies: [color.root]
            )
            do {
                let session = LanguageModelSession()
                let response = try await session.respond(to: fullprompt, schema: schema)
                result.result = response.content.jsonString;
                
            } catch let error as CustomDebugStringConvertible {
                result.result = "{\"error\": \"\(error.debugDescription)\"}";
            } catch let error as LocalizedError {
                let message = error.errorDescription ?? String(describing: error)
                result.result = "{\"error\": \"\(message)\"}";
            } catch {
                result.result = "{\"error\": \"\(error)\"}";
            }
        }
        semaphore.wait()
        return result.result
    }
    return "";
}

