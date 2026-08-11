xLights Controller Vendor Program


*** DRAFT — NOT YET IN EFFECT ***
This document is circulating for feedback. Nothing here is final, and no fees
are being collected under it yet. Dates, prices and details may change before
it is adopted. If you have feedback, we would rather hear it now than after
it is published.



WHY THIS EXISTS

xLights supports over 160 controller models from 25 vendors. That support is not
automatic - every controller in the list represents work by the xLights
development team: a hardware definition describing your ports, protocols and
limits, and in most cases a hand-written upload driver that speaks your
controller's configuration protocol. There is roughly 21,000 lines of
vendor-specific protocol code in xLights today, and it gets re-tested every time
a vendor ships a firmware change.

All of that is written and maintained by volunteers, and given away.

Prop and model vendors have contributed to xLights for years through the model
download program (see README_VENDORS.txt). Controller vendors get at least as
much benefit from xLights integration and, with one exception, have contributed
nothing in the last three years. This program is about correcting that
imbalance - not about making xLights commercial.

xLights remains free and open source. Users never pay for anything described
here.



WHAT YOU GET

There are three levels of integration. Each includes everything below it.


  LEVEL 0 - LISTED

  Your controller appears in the xLights Setup tab by vendor, model and
  variant. xLights then knows your hardware and works with it properly:

    - correct port counts, channel limits and universe limits
    - your supported pixel, serial and input protocols, with anything invalid
      rejected before the user gets confused
    - smart remote support, pixel grouping, zig-zag, null pixels, brightness and
      gamma ranges - whatever your hardware actually does
    - automatic channel layout and sizing across your ports
    - the controller visualiser, so users can see and assign models per port
    - network discovery, so users find your controller instead of typing IPs

  Practical effect: when a user misconfigures your controller, xLights tells
  them - your support inbox doesn't.


  LEVEL 1 - CONFIGURATION UPLOAD

  Users can push their input and output configuration straight from xLights to
  your controller. No re-typing port assignments into your web UI, no
  transcription errors, no support tickets from mismatched channel ranges.

  This is the single most requested controller feature in xLights, and it is the
  one that requires us to implement and maintain your protocol.


  LEVEL 2 - SEQUENCE UPLOAD

  Users can upload sequences, media and playlists to your controller directly
  from xLights - FPP Connect and equivalent SD-card/export paths.

  The primary use case is remote mode. Your controller holds its own copy of the
  sequence data and plays it locally, staying aligned with the rest of the
  display via sync packets, instead of having every channel streamed to it over
  the network for the whole show.

  That matters to your customers more than it might sound:

    - Show size stops being limited by network bandwidth. Streaming every
      channel to every controller is what puts a ceiling on large displays;
      local playback removes it.
    - Playback is immune to network hiccups. A dropped packet loses a sync
      correction, not a frame of your show.
    - No computer has to sit there streaming all season.

  Sequence upload is how the show data gets onto those controllers in the first
  place. Without it, remote mode means the user copying files by hand for every
  controller, every time they change a sequence - which in practice means they
  buy a controller that xLights can upload to instead.

  Standalone SD-card playback is supported by the same level, but remote mode is
  the case that drives the demand.



WHAT IT COSTS

  ONE-TIME

    New vendor listing (includes your first controller model) .... $500
    Each additional controller model ............................. $250
    Five or more models submitted together, each beyond the first  $150

  ANNUAL

    Level 0 - Listed ............................................. $0
    Level 1 - Configuration Upload ............................... $600/year
    Level 2 - Sequence Upload (includes Level 1) ................. $1,800/year

A "model" is one controller product. Its variants - port-count options, serial
options, expansion configurations - are all included at no extra charge.

For comparison, prop vendors in the model download program pay $600 to $1,800
per year. No controller vendor pays more than the largest prop vendor already
does.


WHAT IS ALWAYS FREE

We do not want a fee structure that discourages you from keeping your
information accurate. You are never charged for:

    - adding, removing or correcting variants of a model already listed
    - correcting port counts, protocol lists, channel limits or any other
      detail of an existing definition
    - firmware changes that alter your controller's capabilities
    - renames, rebadges and alternate names for a model already listed
    - anything we got wrong

You pay once when a genuinely new controller joins the catalogue. Keeping it
correct after that costs nothing, forever.



OPEN SOURCE, DIY AND HOBBY PROJECTS

If your project has gained visible traction and is not commercially driven,
contact us. We will consider adding it at whatever level the hardware supports,
at no charge.

This is deliberately not a loophole for commercial vendors, and it is
deliberately not a barrier for the DIY community that much of this hobby is
built on. Roughly what we look for:

    - a real user base - people other than you are building and running it
    - complete, buildable design files under an open license, with genuine
      project history
    - sustained over time, not published last month
    - the normal way to get the hardware is to build it yourself, rather than
      buy a finished product from a company

Selling bare boards at cost, running a group buy, or shipping the occasional kit
does not disqualify you. That is normal practice here.

If a company later forms around the project and starts selling assembled units
at retail, come back and talk to us - that is a different conversation, not an
audit.

Open source firmware platforms themselves - FPP, WLED, ESPixelStick - are
permanently included at no charge. Commercial hardware built on those platforms
is a separate listing that participates on its own.


FPP-BASED CAPES AND HATS

If your cape's EEPROM is signed through the FPP project's store, xLights treats
it as Level 2 entitled at no charge - including capes your customers had signed
themselves.

FPP is a sister project. Money that went into FPP went into this ecosystem, and
we are not going to charge twice for the same thing. DIY builders who get their
EEPROMs signed for free by FPP get the same treatment automatically.



IF YOU CHOOSE NOT TO PARTICIPATE

We would rather say this plainly than have you discover it later.

Your controller stays listed. It remains selectable in Setup, users keep the
port validation, auto-layout, visualiser and discovery, and existing installs
keep working. Nothing your customers own stops functioning.

What is not enabled is upload. Users configure your controller the way they did
before xLights supported upload - through your own web interface. When they try
to upload, xLights tells them the feature isn't enabled for your brand and
suggests they ask you about it.

That is the whole mechanism. It is enforced in the application rather than being
an honour system, but it is deliberately limited to convenience features, and
nothing about a user's show breaks.



GOOD FAITH PARTICIPATION

This program is a voluntary arrangement between us and you. xLights is open
source and you remain free to do anything its license permits - nothing here
restricts your rights to the software. What we are reserving is the right to
decide who we work with.

Participation may be reduced to Level 0, or ended entirely, at the discretion of
the xLights team, if a vendor:

  - distributes modified builds of xLights, or tools that patch xLights, in
    order to bypass the levels described here
  - misrepresents their participation status to customers
  - provides false information to obtain a listing, a level, or the open
    source / DIY exemption
  - conducts a sustained campaign against the project while relying on xLights
    integration to sell hardware

Because we would rather over-explain than have anyone worry, here is what is
explicitly NOT on that list:

  - Criticising xLights, publicly or privately. Tell us we got something wrong.
    Tell your customers we got something wrong. We would rather know.
  - Filing bugs, complaining about our release schedule, or disagreeing with
    this policy - including in public.
  - Building and shipping your own configuration tools, uploaders or utilities.
    That is your product and your business, and writing your own is the
    honest alternative to paying us to write one.
  - Supporting, recommending or selling other sequencing software.

None of those affect your standing. We are not asking anyone to be an advocate.
We are asking you not to actively undermine the thing you are benefiting from.

If we ever do act on this, we will raise it with you first and give you a chance
to respond. Removing an existing controller definition outright punishes your
customers more than it punishes you, so it is a last resort and not a first
response.



WHAT WE NEED FROM YOU

  To get listed, we need enough detail to describe your hardware accurately:
  port counts, channel and universe limits, supported protocols, smart remote
  support, any per-port capabilities, and the variants you ship. Submitting this
  as a working definition file yourself is welcome and speeds things up
  considerably.

  A technical contact. When your firmware changes in a way that affects xLights,
  we need to hear it from you rather than from users filing bugs.

  For Level 1 and Level 2, a protocol. The fee covers enabling and maintaining
  an integration - it does not cover writing a new upload driver from scratch
  for a protocol we have never seen. If your controller needs new driver work,
  either contribute it as a pull request you commit to maintaining, or talk to
  us about it separately. This is the most expensive thing we give away and we
  would rather be up front about it.

  Hardware, ideally. It is very difficult to build and test an upload driver
  against a controller nobody on the team owns.



TIMELINE

  Vendors already listed in xLights owe nothing for their existing models, and
  get a full season's notice before upload levels are enforced. The $250
  per-model fee applies to new models submitted after this document is
  published.

  Vendors not currently listed pay from the date this document is published.

  If you are planning a new controller and want it supported in xLights, talk to
  us early. Lead time matters more than money here - a definition written from a
  spec sheet three weeks before your launch is how bad integrations happen.



COMMON QUESTIONS

  Do my customers have to pay anything?
    No. Never. xLights is free and stays free for users.

  Isn't xLights open source? Why is anyone paying?
    The software is free. The integration work is labour, and it is currently
    unpaid. Prop vendors already contribute on exactly this basis.

  What if I go out of business?
    Your customers keep everything. We mark the vendor as legacy and it stays
    working. We are not in the business of stranding users.

  I already contribute code to xLights.
    Then let's talk. Maintaining your own driver is real contribution and we
    would rather have it than the money.

  Can I pay for listing but not upload?
    Yes, and that is a legitimate place to be. Level 0 has no annual fee.

  I sell a rebadged version of another vendor's controller.
    A rebadge under your own name is a new listing. Reselling another vendor's
    participating hardware unchanged is not.

  Who receives the money?
    xLights LLC, the same entity that receives the model vendor payments.



CONTACT

Contact any of the xLights developers to discuss joining, to ask whether your
project qualifies for the open source / DIY route, or to tell us this document
gets something wrong.
