Vendor Fees and process.

Adding props to the xLights downloads requires a yearly "donation" to the xLights
development team via the normal PayPal donation buttons/process.
Example:  https://www.paypal.com/donate/?hosted_button_id=BB6366BT755H6

The fee is per model as follows:
    50 and under models: $12/model    ($600/year for 50)
    100 and under models: $11/model   ($1100/year for 100)
    150 and under models: $10/model   ($1500/year for 150)
    Unlimitted models: $1800/year     ($1800/year for >150)

Once the donation has been submitted and initial version your XML file is in place, contact
one of the xLights developers to have your company added to the vendor list.   Since xLights
downloads the list on demand, the change would be visible by customers almost immediately.
(subject to download caches)



Vendor Model XML File:
The XML file is relatively straight forward.  There is a starting template at:
https://github.com/xLightsSequencer/xLights/blob/master/download/templates/vendor_model_example.xml

For the most part, there are three sections:
1) vendor - this section provides information about your company.   Most settings are optional,
but including things like your website, logo, etc... is encouraged.

2) categories - this is a "tree" structure of categories and is how they would be displayed
in the model download dialog in xLights.   It is up to you to organize the categories in ways
that make sense to your customers.  However, common "top level" categories like "Christmas" and
"Halloween" are encouraged to make it easier for xLights users to investigate options for
their displays.   The "id" in the category is important as that's how the models will be assigned
to them.

3) models - a model node for each model.  Most items are optional, but the more information you
provide, the better xLights can show the option to the user.   Note:  xLights will enforce the
model count.  If you have paid for 50 models, but put 200 in the file, only the first 50
will be displayed to the user.



There are a couple of tools to help create/edit the XML file.
* There is an Excel file at:
https://github.com/xLightsSequencer/xLights/raw/refs/heads/master/download/vendor_xml_generator.xlsm
which can be used to manage it.   That requires Microsoft Excel and all of it's scripting stuff
enabled.

* Once added to the xlights_vendors.xml, you can use:
https://kulplights.com/vendorxml/
to manipulate the xml and then hit the "Generate XML" button to get a new XML file.  This supports
basic editing of models, drag/dop to arrange the tree, etc...  If there is sufficient interest,
this could be extended to save the XML directly on kulplights.com and hosted from there.
If interested in the code, it's a PHP script that can likely be added to FPP or other
PHP hosting solution.  https://kulplights.com/vendorxml/vendorxml.zip


Using a raw text editor to edit the file is perfectly fine or using custom scripts to generate
it from your inventory system or store is definitely doable.
