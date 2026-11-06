#pragma once

// Trimmed copies of resources/controllers/*.xcontroller: the abstract variants plus the
// controllers the vendor upload tests configure. Kept in the test so it does not depend on
// where the test binary runs from.

inline const char* kCaps_advatek = R"XML(<Vendor Name="Advatek">
    <AbstractVariant Name="PLMK3">
        <SupportsPixelPortCommonSettings/>
        <SupportsUpload/>
        <MaxSerialPortChannels>512</MaxSerialPortChannels>
        <MaxPixelPortChannels>3060</MaxPixelPortChannels>
        <MaxStartNulls>50</MaxStartNulls>
        <MaxGroup>50</MaxGroup>
        <MinGroup>1</MinGroup>
        <SupportsAutoLayout/>
        <NeedsFullUniverseForSerial/>
        <SupportsFullxLightsControl/>
        <AllInputUniversesMustBeSameSize/>
        <AllInputUniversesMustBe510/>
        <PixelProtocols>
            <Protocol>ws2811</Protocol>
            <Protocol>tm1803</Protocol>
            <Protocol>tm1804</Protocol>
            <Protocol>tm1809</Protocol>
            <Protocol>tm1814</Protocol>
            <Protocol>tm1914</Protocol>
            <Protocol>ws2801</Protocol>
            <Protocol>tls3001</Protocol>
            <Protocol>lpd6803</Protocol>
            <Protocol>lpd8806</Protocol>
            <Protocol>apa102</Protocol>
            <Protocol>apa104</Protocol>
            <Protocol>apa109</Protocol>
            <Protocol>dmx512</Protocol>
            <Protocol>sm16703</Protocol>
            <Protocol>sm16704</Protocol>
            <Protocol>sm16714</Protocol>
            <Protocol>sm16716</Protocol>
            <Protocol>mbi6020</Protocol>
            <Protocol>my9231</Protocol>
            <Protocol>my9221</Protocol>
            <Protocol>my9291</Protocol>
            <Protocol>sk6812</Protocol>
            <Protocol>sk6813</Protocol>
            <Protocol>sk6822</Protocol>
            <Protocol>sk9822</Protocol>
            <Protocol>ucs1903</Protocol>
            <Protocol>ucs2903</Protocol>
            <Protocol>ucs2904</Protocol>
            <Protocol>ucs9812</Protocol>
            <Protocol>gs8208</Protocol>
            <Protocol>p9813</Protocol>
        </PixelProtocols>
        <SerialProtocols>
            <Protocol>dmx</Protocol>
        </SerialProtocols>
        <InputProtocols>
            <Protocol>e131</Protocol>
            <Protocol>artnet</Protocol>
        </InputProtocols>
        <ConfigDriver>PixLite16</ConfigDriver>
    </AbstractVariant>
    <Controller Name="PixLite E16-S Mk3">
		<Variant Name="Normal" Base="Advatek:PLMK3">
			<MaxInputUniverses>96</MaxInputUniverses>
			<MaxPixelPort>16</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
		</Variant>
		<Variant Name="Expanded" Base="Advatek:PLMK3">
			<MaxInputUniverses>96</MaxInputUniverses>
			<MaxPixelPort>32</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
            <ForceExpanded>true</ForceExpanded>
		</Variant>
    </Controller>
    <AbstractVariant Name="PLMK1">
        <SupportsPixelPortCommonSettings/>
        <SupportsUpload/>
        <MaxSerialPortChannels>512</MaxSerialPortChannels>
        <MaxPixelPortChannels>2040</MaxPixelPortChannels>
        <SupportsAutoLayout/>
        <MaxStartNulls>10</MaxStartNulls>
        <MaxGroup>340</MaxGroup>
        <MinGroup>1</MinGroup>
        <NeedsFullUniverseForSerial/>
        <NoWebUI/>
        <SupportsFullxLightsControl/>
        <AllInputUniversesMustBeSameSize/>
        <AllInputUniversesMustBe510/>
        <PixelProtocols>
            <Protocol>ws2811</Protocol>
            <Protocol>tm18xx</Protocol>
            <Protocol>tm1803</Protocol>
            <Protocol>tm1804</Protocol>
            <Protocol>tm1809</Protocol>
            <Protocol>ws2801</Protocol>
            <Protocol>tls3001</Protocol>
            <Protocol>lpd6803</Protocol>
            <Protocol>lpd8806</Protocol>
            <Protocol>apa102</Protocol>
            <Protocol>apa109</Protocol>
            <Protocol>sm16716</Protocol>
            <Protocol>mb16020</Protocol>
            <Protocol>my9231</Protocol>
            <Protocol>my9221</Protocol>
            <Protocol>sk6812</Protocol>
            <Protocol>ucs1903</Protocol>
            <Protocol>ucs2903</Protocol>
        </PixelProtocols>
        <SerialProtocols>
            <Protocol>dmx</Protocol>
        </SerialProtocols>
        <InputProtocols>
            <Protocol>e131</Protocol>
            <Protocol>artnet</Protocol>
        </InputProtocols>
        <ConfigDriver>PixLite16</ConfigDriver>
    </AbstractVariant>
    <AbstractVariant Name="PLMK2">
        <SupportsPixelPortCommonSettings/>
        <SupportsUpload/>
        <MaxSerialPortChannels>512</MaxSerialPortChannels>
        <MaxPixelPortChannels>3060</MaxPixelPortChannels>
        <MaxStartNulls>10</MaxStartNulls>
        <MaxGroup>340</MaxGroup>
        <MinGroup>1</MinGroup>
        <NoWebUI/>
        <SupportsAutoLayout/>
        <SupportsPixelZigZag/>
        <MaxZigZag>768</MaxZigZag>
        <NeedsFullUniverseForSerial/>
        <SupportsFullxLightsControl/>
        <AllInputUniversesMustBeSameSize/>
        <AllInputUniversesMustBe510/>
        <PixelProtocols>
            <Protocol>ws2811</Protocol>
            <Protocol>tm18xx</Protocol>
            <Protocol>tm1803</Protocol>
            <Protocol>tm1804</Protocol>
            <Protocol>tm1809</Protocol>
            <Protocol>ws2801</Protocol>
            <Protocol>tls3001</Protocol>
            <Protocol>lpd6803</Protocol>
            <Protocol>lpd8806</Protocol>
            <Protocol>apa102</Protocol>
            <Protocol>apa109</Protocol>
            <Protocol>dmx512</Protocol>
            <Protocol>sm16716</Protocol>
            <Protocol>mb16020</Protocol>
            <Protocol>my9231</Protocol>
            <Protocol>my9221</Protocol>
            <Protocol>sk6812</Protocol>
            <Protocol>ucs1903</Protocol>
            <Protocol>ucs2903</Protocol>
        </PixelProtocols>
        <SerialProtocols>
            <Protocol>dmx</Protocol>
        </SerialProtocols>
        <InputProtocols>
            <Protocol>e131</Protocol>
            <Protocol>artnet</Protocol>
        </InputProtocols>
        <ConfigDriver>PixLite16</ConfigDriver>
    </AbstractVariant>
	<Controller Name="PixLite 16 MkII">
		<Variant Name="Normal" Base="Advatek:PLMK2">
			<MaxInputUniverses>96</MaxInputUniverses>
			<MaxPixelPort>16</MaxPixelPort>
			<MaxSerialPort>4</MaxSerialPort>
		</Variant>
		<Variant Name="Expanded" Base="Advatek:PLMK2">
			<MaxInputUniverses>96</MaxInputUniverses>
			<MaxPixelPort>32</MaxPixelPort>
			<MaxSerialPort>4</MaxSerialPort>
            <ForceExpanded>true</ForceExpanded>
		</Variant>
	</Controller>
</Vendor>)XML";

inline const char* kCaps_falcon = R"XML(<Vendor Name="Falcon">
    <AbstractVariant Name="BaseFalconSettings">
        <SupportsUpload/>
        <SupportsInputOnlyUpload/>
		<SupportsPixelPortCommonSettings/>
        <SupportsPixelZigZag/>
        <MaxZigZag>512</MaxZigZag>
		<SupportsFullxLightsControl/>
        <SupportsMultipleSimultaneousOutputProtocols/>
        <SupportsMultipleSimultaneousInputProtocols/>
        <UniversesMustBeInNumericalOrder/>
        <SupportsVirtualStrings/>
        <SupportsAutoLayout/>
		<SupportsRemotes/>
        <SupportsDefaultBrightness/>
        <MaxInputUniverses>96</MaxInputUniverses>
        <MaxSerialPortChannels>512</MaxSerialPortChannels>
        <SerialProtocols>
            <Protocol>dmx</Protocol>
            <Protocol>renard</Protocol>
            <Protocol>pixelnet</Protocol>
        </SerialProtocols>
        <PixelProtocols>
            <Protocol>ws2811</Protocol>
            <Protocol>tm18xx</Protocol>
            <Protocol>lx1203</Protocol>
            <Protocol>ws2801</Protocol>
            <Protocol>tls3001</Protocol>
            <Protocol>lpd6803</Protocol>
            <Protocol>gece</Protocol>
            <Protocol>lpd8806</Protocol>
            <Protocol>apa102</Protocol>
            <Protocol>ucs2903</Protocol>
            <Protocol>tm1814a</Protocol>
        </PixelProtocols>
        <ConfigDriver>Falcon</ConfigDriver>
    </AbstractVariant>
    <AbstractVariant Name="BaseFalconV2Settings" Base="Falcon:BaseFalconSettings">
        <MaxPixelPortChannels>2040</MaxPixelPortChannels>
        <InputProtocols>
            <Protocol>e131</Protocol>
            <Protocol>artnet</Protocol>
        </InputProtocols>
		<FPS40Pixels>680</FPS40Pixels>
    </AbstractVariant>
    <AbstractVariant Name="BaseFalconV3Settings" Base="Falcon:BaseFalconSettings">
        <SupportsSmartRemotes>3</SupportsSmartRemotes>
        <AllSmartRemoteTypesPerPortMustBeSame/>
        <MaxPixelPortChannels>3072</MaxPixelPortChannels>
        <MaxDDPChannels>42000</MaxDDPChannels>
        <SupportsDefaultGamma/>
        <InputProtocols>
            <Protocol>e131</Protocol>
            <Protocol>artnet</Protocol>
            <Protocol>zcpp</Protocol>
            <Protocol>ddp</Protocol>
        </InputProtocols>
		<NeedsDDPInputUpload/>
        <SmartRemoteTypes>
            <Type>falcon_v1</Type>
        </SmartRemoteTypes>
		<FPS40Pixels>680</FPS40Pixels>
    </AbstractVariant>
	<Controller Name="F16V2">
		<Variant Name="No Expansion Boards" Base="Falcon:BaseFalconV2Settings">
			<MaxPixelPort>16</MaxPixelPort>
			<MaxSerialPort>4</MaxSerialPort>
		</Variant>
		<Variant Name="One Expansion Board" Base="Falcon:BaseFalconV2Settings">
			<MaxPixelPort>32</MaxPixelPort>
			<MaxSerialPort>4</MaxSerialPort>
		</Variant>
	</Controller>
	<Controller Name="F16V3">
		<Variant Name="No Expansion Boards" Base="Falcon:BaseFalconV3Settings">
			<MaxPixelPort>16</MaxPixelPort>
			<MaxSerialPort>4</MaxSerialPort>
		</Variant>
		<Variant Name="No Expansion + Serial Port Remotes" Base="Falcon:BaseFalconV3Settings">
			<MaxPixelPort>20</MaxPixelPort>
			<MaxSerialPort>0</MaxSerialPort>
			<NumberOfBanks>2</NumberOfBanks>
            <BankSize>16</BankSize>
		</Variant>
		<Variant Name="One Expansion Board" Base="Falcon:BaseFalconV3Settings">
			<MaxPixelPort>32</MaxPixelPort>
			<MaxSerialPort>4</MaxSerialPort>
			<NumberOfBanks>2</NumberOfBanks>
            <BankSize>16</BankSize>
		</Variant>
		<Variant Name="One Expansion + Serial Port Remotes" Base="Falcon:BaseFalconV3Settings">
			<MaxPixelPort>36</MaxPixelPort>
			<MaxSerialPort>0</MaxSerialPort>
			<NumberOfBanks>3</NumberOfBanks>
            <BankSize>16</BankSize>
		</Variant>
		<Variant Name="Two Expansion Boards" Base="Falcon:BaseFalconV3Settings">
			<MaxPixelPort>48</MaxPixelPort>
			<MaxSerialPort>4</MaxSerialPort>
			<NumberOfBanks>3</NumberOfBanks>
            <BankSize>16</BankSize>
		</Variant>
	</Controller>
    <AbstractVariant Name="BaseV4FalconSettings">
        <SupportsUpload/>
        <SupportsInputOnlyUpload/>
		<SupportsPixelPortCommonSettings/>
        <SupportsPixelZigZag/>
        <MaxZigZag>512</MaxZigZag>
		<SupportsFullxLightsControl/>
        <SupportsMultipleSimultaneousOutputProtocols/>
        <SupportsMultipleSimultaneousInputProtocols/>
        <SupportsVirtualStrings/>
        <SupportsAutoLayout/>
        <SupportsPixelPortEndNullPixels/>
        <MaxStartNulls>20</MaxStartNulls>
        <MaxEndNulls>20</MaxEndNulls>
        <MaxGroup>100</MaxGroup>
        <MinGroup>1</MinGroup>
        <AllSmartRemoteTypesPerPortMustBeSame/>
        <SupportsDefaultBrightness/>
        <SupportsDefaultGamma/>
        <MaxInputUniverses>192</MaxInputUniverses>
        <MaxSerialPortChannels>512</MaxSerialPortChannels>
		<PreferredInputProtocol>DDP</PreferredInputProtocol>
        <SerialProtocols>
            <Protocol>dmx</Protocol>
            <Protocol>renard</Protocol>
        </SerialProtocols>
        <PixelProtocols>
            <Protocol>ws2811</Protocol>
			<Protocol>apa102</Protocol>
			<Protocol>apa109</Protocol>
			<Protocol>lpd6803</Protocol>
			<Protocol>lpd8806</Protocol>
			<Protocol>sm16716</Protocol>
			<Protocol>tls3001</Protocol>
			<Protocol>tm1814</Protocol>
			<Protocol>tm1829</Protocol>
			<Protocol>ucs8903</Protocol>
			<Protocol>ucs8903 16 bit</Protocol>
			<Protocol>ucs8904</Protocol>
			<Protocol>ucs8904 16 bit</Protocol>
			<Protocol>ws2801</Protocol>
			<Protocol>ws2811</Protocol>
			<Protocol>ws2811 slow</Protocol>
			<Protocol>dmx512p</Protocol>
			<Protocol>dmx512p-4</Protocol>
			<Protocol>sj1221</Protocol>
			<Protocol>sm16825</Protocol>
			<Protocol>sm16825 16 bit</Protocol>
		</PixelProtocols>
        <InputProtocols>
            <Protocol>e131</Protocol>
            <Protocol>artnet</Protocol>
            <Protocol>ddp</Protocol>
            <Protocol>zcpp</Protocol>
        </InputProtocols>
        <SmartRemoteTypes>
            <Type>falcon_v2</Type>
        </SmartRemoteTypes>
        <ConfigDriver>Falcon</ConfigDriver>
		<FPS40Pixels>704</FPS40Pixels>
    </AbstractVariant>
	<Controller Name="F16V4">
		<Variant Name="16 Local Ports" Base="Falcon:BaseV4FalconSettings">
			<MaxPixelPort>16</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxPixelPortChannels>3072</MaxPixelPortChannels>
            <v4BoardMode>0</v4BoardMode>
		</Variant>
		<Variant Name="16 Local Ports + 2 Smart Receiver Chains" Base="Falcon:BaseV4FalconSettings">
			<MaxPixelPort>24</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxPixelPortChannels>3072</MaxPixelPortChannels>
            <v4BoardMode>1</v4BoardMode>
			<SupportsRemotes/>
			<SupportsSmartRemotes>6</SupportsSmartRemotes>
		</Variant>
		<Variant Name="16 + 16 Local Ports" Base="Falcon:BaseV4FalconSettings">
			<MaxPixelPort>32</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxPixelPortChannels>3072</MaxPixelPortChannels>
            <v4BoardMode>2</v4BoardMode>
		</Variant>
		<Variant Name="16 + 16 Local Ports + 2 Smart Receiver Chains" Base="Falcon:BaseV4FalconSettings">
			<MaxPixelPort>40</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxPixelPortChannels>2112</MaxPixelPortChannels>
            <v4BoardMode>3</v4BoardMode>
			<SupportsRemotes/>
			<SupportsSmartRemotes>6</SupportsSmartRemotes>
		</Variant>
		<Variant Name="16 Local Ports + 4 Smart Receiver Chains" Base="Falcon:BaseV4FalconSettings">
			<MaxPixelPort>32</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxPixelPortChannels>3072</MaxPixelPortChannels>
            <v4BoardMode>4</v4BoardMode>
			<SupportsRemotes/>
			<SupportsSmartRemotes>6</SupportsSmartRemotes>
		</Variant>
		<Variant Name="16 Local Ports + 4 + 2 Smart Receiver Chains" Base="Falcon:BaseV4FalconSettings">
			<MaxPixelPort>40</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxPixelPortChannels>2112</MaxPixelPortChannels>
            <v4BoardMode>5</v4BoardMode>
			<SupportsRemotes/>
			<SupportsSmartRemotes>6</SupportsSmartRemotes>
		</Variant>
		<Variant Name="16 + 16 + 16 Local Ports" Base="Falcon:BaseV4FalconSettings">
			<MaxPixelPort>48</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxPixelPortChannels>2112</MaxPixelPortChannels>
            <v4BoardMode>6</v4BoardMode>
		</Variant>
		<Variant Name="16 + 16 Local Ports + 4 Smart Receiver Chains" Base="Falcon:BaseV4FalconSettings">
			<MaxPixelPort>48</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxPixelPortChannels>2112</MaxPixelPortChannels>
            <v4BoardMode>7</v4BoardMode>
			<SupportsRemotes/>
			<SupportsSmartRemotes>6</SupportsSmartRemotes>
		</Variant>
		<Variant Name="16 + 4 Smart Receiver Chains + 16 Local Ports" Base="Falcon:BaseV4FalconSettings">
			<MaxPixelPort>48</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxPixelPortChannels>2112</MaxPixelPortChannels>
            <v4BoardMode>8</v4BoardMode>
			<SupportsRemotes/>
			<SupportsSmartRemotes>6</SupportsSmartRemotes>
		</Variant>
		<Variant Name="16 Local Ports + 4 + 4 Smart Receiver Chains" Base="Falcon:BaseV4FalconSettings">
			<MaxPixelPort>48</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxPixelPortChannels>2112</MaxPixelPortChannels>
      <v4BoardMode>9</v4BoardMode>
			<SupportsRemotes/>
			<SupportsSmartRemotes>6</SupportsSmartRemotes>
		</Variant>
	</Controller>
</Vendor>)XML";

inline const char* kCaps_hinkspix = R"XML(<Vendor Name="HinksPix">
	<Controller Name="EasyLights Pix16">
		<Variant Name="">
			<SupportsPixelPortBrightness/>
			<SupportsPixelPortGamma/>
			<SupportsPixelPortNullPixels/>
			<SupportsPixelPortDirection/>
			<SupportsPixelPortColourOrder/>
			<DMXAfterPixels/>
			<SupportsUpload/>
			<SupportsRemotes/>
			<MaxInputUniverses>65</MaxInputUniverses>
			<MaxPixelPort>16</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxSerialPortChannels>512</MaxSerialPortChannels>
			<MaxPixelPortChannels>2040</MaxPixelPortChannels>
			<SupportsAutoLayout/>
			<SupportsFullxLightsControl/>
			<SupportsDefaultBrightness/>
			<DisableMonitoring/>
			<PixelProtocols>
				<Protocol>ws2811</Protocol>
				<Protocol>ws2801</Protocol>
				<Protocol>tls3001</Protocol>
				<Protocol>apa102</Protocol>
			</PixelProtocols>
			<SerialProtocols>
				<Protocol>dmx</Protocol>
			</SerialProtocols>
			<InputProtocols>
				<Protocol>e131</Protocol>
				<Protocol>artnet</Protocol>
				<Protocol>ddp</Protocol>
			</InputProtocols>
			<SupportsMultipleSimultaneousOutputProtocols/>
			<ConfigDriver>HinksPix</ConfigDriver>
		</Variant>
	</Controller>
	<Controller Name="PRO V1/V2">
		<Variant Name="">
			<SupportsPixelPortBrightness/>
			<SupportsPixelPortGamma/>
			<SupportsPixelPortNullPixels/>
			<SupportsPixelPortDirection/>
			<SupportsPixelPortColourOrder/>
			<SupportsUniversePerString/>
			<SupportsUnPackedModels/>
			<DMXAfterPixels/>
			<SupportsRemotes/>
			<SupportsSmartRemotes>16</SupportsSmartRemotes>
			<SupportsUpload/>
			<MaxInputUniverses>402</MaxInputUniverses>
			<MaxPixelPort>48</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxSerialPortChannels>512</MaxSerialPortChannels>
			<MaxPixelPortChannels>2040</MaxPixelPortChannels>
			<SupportsAutoLayout/>
			<SupportsFullxLightsControl/>
			<SupportsDefaultBrightness/>
			<DisableMonitoring/>
			<PixelProtocols>
				<Protocol>ws2811</Protocol>
			</PixelProtocols>
			<SerialProtocols>
				<Protocol>dmx</Protocol>
			</SerialProtocols>
			<InputProtocols>
				<Protocol>e131</Protocol>
				<Protocol>artnet</Protocol>
			</InputProtocols>
			<SmartRemoteTypes>
				<Type>hinkspix_4</Type>
				<Type>hinkspix_16</Type>
				<Type>hinkspix_16ac</Type>
			</SmartRemoteTypes>
			<AltNames>
				<AltName>PRO</AltName>
			</AltNames>
			<SupportsMultipleSimultaneousOutputProtocols/>
			<ConfigDriver>HinksPix</ConfigDriver>
		</Variant>
	</Controller>
	<Controller Name="PRO V3">
		<Variant Name="">
			<SupportsPixelPortBrightness/>
			<SupportsPixelPortGamma/>
			<SupportsPixelPortNullPixels/>
			<SupportsPixelPortDirection/>
			<SupportsPixelPortColourOrder/>
			<SupportsUniversePerString/>
			<DMXAfterPixels/>
			<SupportsRemotes/>
			<SupportsSmartRemotes>16</SupportsSmartRemotes>
			<SupportsUpload/>
			<MaxInputUniverses>684</MaxInputUniverses>
			<MaxPixelPort>80</MaxPixelPort>
			<MaxSerialPort>1</MaxSerialPort>
			<MaxSerialPortChannels>512</MaxSerialPortChannels>
			<MaxPixelPortChannels>3072</MaxPixelPortChannels>
			<SupportsAutoLayout/>
			<SupportsFullxLightsControl/>
			<SupportsDefaultBrightness/>
			<DisableMonitoring/>
			<PixelProtocols>
				<Protocol>ws2811</Protocol>
			</PixelProtocols>
			<SerialProtocols>
				<Protocol>dmx</Protocol>
			</SerialProtocols>
			<InputProtocols>
				<Protocol>e131</Protocol>
				<Protocol>artnet</Protocol>
				<Protocol>ddp</Protocol>
			</InputProtocols>
			<SmartRemoteTypes>
				<Type>hinkspix_4</Type>
				<Type>hinkspix_16</Type>
				<Type>hinkspix_16ac</Type>
			</SmartRemoteTypes>
			<AltNames>
				<AltName>PRO80</AltName>
			</AltNames>
			<SupportsMultipleSimultaneousOutputProtocols/>
			<ConfigDriver>HinksPix</ConfigDriver>
		</Variant>
	</Controller>
</Vendor>)XML";

inline const char* kCaps_ilightthat = R"XML(<Vendor Name="ILightThat">
    <AbstractVariant Name="BaseBaldrickSettings">
		<SupportsPixelPortBrightness/>
		<SupportsPixelPortGamma/>
		<SupportsPixelPortColourOrder/>
        <SupportsUpload/>
        <MaxInputUniverses>36</MaxInputUniverses>
        <MaxSerialPortChannels>0</MaxSerialPortChannels>
        <MaxPixelPortChannels>2250</MaxPixelPortChannels>
        <AllInputUniversesMustBeSameSize/>
        <UniversesMustBeSequential/>
        <SupportsFullxLightsControl/>
        <SupportsDefaultBrightness/>
        <SupportsDefaultGamma/>
        <SupportsAutoLayout/>
        <PreferredInputProtocol>DDP</PreferredInputProtocol>
        <PixelProtocols>
            <Protocol>ws2811</Protocol>
        </PixelProtocols>
        <InputProtocols>
            <Protocol>ddp</Protocol>
            <Protocol>artnet</Protocol>
            <Protocol>e131</Protocol>
        </InputProtocols>
        <SerialProtocols>
            <Protocol>dmx</Protocol>
        </SerialProtocols>
        <ConfigDriver>ILightThat</ConfigDriver>
    </AbstractVariant>
    <Controller Name="Baldrick8">
		<Variant Name="" Base="ILightThat:BaseBaldrickSettings">
			<MaxPixelPort>8</MaxPixelPort>
			<MaxSerialPort>0</MaxSerialPort>
		</Variant>
	</Controller>
</Vendor>)XML";

inline const char* kCaps_j1sys = R"XML(<Vendor Name="J1Sys">
	<Controller Name="P2">
		<Variant Name="">
            <SupportsAutoLayout/>
			<SupportsPixelPortCommonSettings/>
			<SupportsUpload/>
			<MaxInputUniverses>8</MaxInputUniverses>
			<MaxPixelPort>2</MaxPixelPort>
			<MaxSerialPort>0</MaxSerialPort>
			<MaxSerialPortChannels>0</MaxSerialPortChannels>
			<MaxPixelPortChannels>3072</MaxPixelPortChannels>
			<PixelProtocols>
				<Protocol>ws2811</Protocol>
				<Protocol>tm18xx</Protocol>
				<Protocol>ws2801</Protocol>
				<Protocol>tls3001</Protocol>
				<Protocol>lpd6803</Protocol>
			</PixelProtocols>
			<SerialProtocols/>
			<InputProtocols>
				<Protocol>e131</Protocol>
				<Protocol>artnet</Protocol>
			</InputProtocols>
			<SupportsMultipleSimultaneousOutputProtocols/>
            <ConfigDriver>J1Sys</ConfigDriver>
		</Variant>
	</Controller>
	<Controller Name="P12S">
		<Variant Name="">
            <SupportsAutoLayout/>
			<SupportsPixelPortCommonSettings/>
			<SupportsUpload/>
			<MaxInputUniverses>96</MaxInputUniverses>
			<MaxPixelPort>12</MaxPixelPort>
			<MaxSerialPort>2</MaxSerialPort>
			<MaxSerialPortChannels>512</MaxSerialPortChannels>
			<MaxPixelPortChannels>1020</MaxPixelPortChannels>
			<PixelProtocols>
				<Protocol>ws2811</Protocol>
				<Protocol>tm18xx</Protocol>
				<Protocol>ws2801</Protocol>
				<Protocol>tls3001</Protocol>
				<Protocol>lpd6803</Protocol>
			</PixelProtocols>
			<SerialProtocols>
				<Protocol>dmx</Protocol>
				<Protocol>renard</Protocol>
			</SerialProtocols>
			<InputProtocols>
				<Protocol>e131</Protocol>
				<Protocol>artnet</Protocol>
			</InputProtocols>
			<SupportsMultipleSimultaneousOutputProtocols/>
            <ConfigDriver>J1Sys</ConfigDriver>
		</Variant>
	</Controller>
</Vendor>)XML";

inline const char* kCaps_experience = R"XML(<Vendor Name="Experience Lights">
	<AbstractVariant Name="BaseExperienceSettings">
		<SupportsPixelPortBrightness/>
		<SupportsPixelPortGamma/>
		<SupportsPixelPortNullPixels/>
		<SupportsPixelPortDirection/>
		<SupportsPixelPortColourOrder/>
		<SupportsPixelPortGrouping/>
		<SupportsRemotes/>
		<SupportsSmartRemotes>6</SupportsSmartRemotes>
		<AllSmartRemoteTypesPerPortMustBeSame/>
		<SupportsPixelPortEndNullPixels/>
		<SupportsUpload/>
		<MaxInputUniverses>64</MaxInputUniverses>
		<MaxPixelPort>24</MaxPixelPort>
		<MaxSerialPort>2</MaxSerialPort>
		<MaxSerialPortChannels>512</MaxSerialPortChannels>
		<MaxPixelPortChannels>6000</MaxPixelPortChannels>
		<SupportsVirtualStrings/>
		<SupportsAutoLayout/>
		<SupportsFullxLightsControl/>
		<SupportsDefaultBrightness/>
		<SupportsDefaultGamma/>
		<PreferredInputProtocol>DDP</PreferredInputProtocol>
		<PixelProtocols>
			<Protocol>ws2811</Protocol>
		</PixelProtocols>
		<SerialProtocols>
			<Protocol>dmx</Protocol>
		</SerialProtocols>
		<InputProtocols>
			<Protocol>e131</Protocol>
			<Protocol>ddp</Protocol>
			<Protocol>artnet</Protocol>
		</InputProtocols>
		<SmartRemoteTypes>
			<Type>falcon_v2</Type>
		</SmartRemoteTypes>
		<DisableMonitoring/>
		<ConfigDriver>Experience</ConfigDriver>
	</AbstractVariant>
	<Controller Name="Genius Pixel 16">
		<Variant Name="" Base="Experience Lights:BaseExperienceSettings">
			<MaxPixelPort>24</MaxPixelPort>
			<MaxSerialPort>2</MaxSerialPort>
		</Variant>
	</Controller>
	<Controller Name="Genius Pro 16 Port">
		<Variant Name="" Base="Experience Lights:BaseExperienceSettings">
			<MaxPixelPort>24</MaxPixelPort>
			<MaxSerialPort>2</MaxSerialPort>
			<eFuses>true</eFuses>
		</Variant>
	</Controller>
</Vendor>)XML";

inline const char* kCaps_powerdmx = R"XML(<Vendor Name="PowerDMX">
    <AbstractVariant Name="BasePowerDMX">
        <SupportsUpload/>
        <SupportsAutoUpload/>
        <SupportsAutoLayout/>
        <MaxInputUniverses>1024</MaxInputUniverses>
        <MaxPixelPortChannels>4096</MaxPixelPortChannels>
        <ConfigDriver>PowerDMX</ConfigDriver>
        <InputProtocols>
            <Protocol>ddp</Protocol>
            <Protocol>e131</Protocol>
        </InputProtocols>
        <PixelProtocols>
            <Protocol>ws2811</Protocol>
        </PixelProtocols>
    </AbstractVariant>
    <Controller Name="PowerDMX-7">
        <Variant Name="" ID="PowerDMX-7" Base="PowerDMX:BasePowerDMX">
            <MaxPixelPort>7</MaxPixelPort>
        </Variant>
    </Controller>
</Vendor>)XML";

inline const char* kCaps_wled = R"XML(<Vendor Name="WLED">
    <AbstractVariant Name="BaseWLEDSettings">
        <OpenSourceFirmware/>
        <SupportsPixelPortDirection/>
        <SupportsPixelPortColourOrder/>
        <SupportsPixelPortNullPixels/>
        <SupportsUpload/>
        <MaxInputUniverses>9</MaxInputUniverses>
        <MaxSerialPortChannels>0</MaxSerialPortChannels>
        <MaxPixelPortChannels>2400</MaxPixelPortChannels>
        <MaxPacing>30</MaxPacing>
        <AllInputUniversesMustBeSameSize/>
        <UniversesMustBeSequential/>
        <SupportsAutoLayout/>
        <DDPStartsAtOne/>
        <SupportsFullxLightsControl/>
        <SupportsDefaultBrightness/>
        <SupportsMultipleSimultaneousOutputProtocols/>
        <PixelProtocols>
            <Protocol>ws2811</Protocol>
            <Protocol>sk6812rgbw</Protocol>
            <Protocol>apa109</Protocol>
            <Protocol>tm1814</Protocol>
            <Protocol>tm1829</Protocol>
            <Protocol>ucs8903</Protocol>
            <Protocol>ucs8904</Protocol>
            <Protocol>ws2801</Protocol>
            <Protocol>apa102</Protocol>
            <Protocol>lpd8806</Protocol>
            <Protocol>p9813</Protocol>
            <Protocol>lpd6803</Protocol>
        </PixelProtocols>
        <SerialProtocols/>
        <InputProtocols>
            <Protocol>e131</Protocol>
            <Protocol>ddp</Protocol>
            <Protocol>artnet</Protocol>
        </InputProtocols>
        <ConfigDriver>WLED</ConfigDriver>
    </AbstractVariant>
    <AbstractVariant Name="ESP32WLEDSettings" Base="WLED:BaseWLEDSettings">
        <MaxInputUniverses>24</MaxInputUniverses>
        <MaxPixelPortChannels>3600</MaxPixelPortChannels>
    </AbstractVariant>
    <Controller Name="WLED">
        <Variant Name="Generic ESP8266" Base="WLED:BaseWLEDSettings">
            <MaxPixelPort>3</MaxPixelPort>
            <MaxSerialPort>0</MaxSerialPort>
            <Port1>2</Port1>
            <Port2>1</Port2>
            <Port3>3</Port3>
        </Variant>
        <Variant Name="Generic ESP32" Base="WLED:ESP32WLEDSettings">
            <MaxPixelPort>8</MaxPixelPort>
            <MaxSerialPort>0</MaxSerialPort>
            <Port1>2</Port1>
            <Port2>13</Port2>
            <Port3>12</Port3>
            <Port4>14</Port4>
            <Port5>27</Port5>
            <Port6>26</Port6>
            <Port7>25</Port7>
            <Port8>33</Port8>
        </Variant>
    </Controller>
</Vendor>)XML";
