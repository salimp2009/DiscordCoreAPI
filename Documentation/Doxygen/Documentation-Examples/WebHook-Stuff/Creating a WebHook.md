Creating a WebHook {#creatingawebhook}
============
- Execute the, `DiscordCoreAPI::WebHooks::createWebHookAsync()` function, while passing in a value of type `DiscordCoreAPI::CreateWebHookData`, with a return value of type `auto` or `DiscordCoreAPI::WebHook`.
- Call the function with `.get()` added to the end in order to wait for the results now.

```cpp
/// Test.hpp-Header for the "test" command.
/// https://github.com/RealTimeChris/DiscordCoreAPI

#pragma once

#include <Index.hpp>

namespace DiscordCoreAPI {

	class Test : public DiscordCoreAPI::BaseFunction {
	  public:
		Test() {
			this->commandName = "test";
			this->helpDescription = "Testing purposes!";
			DiscordCoreAPI::EmbedData msgEmbed { };
			msgEmbed.setDescription("------\nSimply enter !test or /test!\n------");
			msgEmbed.setTitle("__**Test Usage:**__");
			msgEmbed.setTimeStamp(getTimeAndDate());
			msgEmbed.setColor("FeFeFe");
			this->helpEmbed = msgEmbed;
		}

		std::unique_ptr<DiscordCoreAPI::BaseFunction> create() {
			return std::make_unique<Test>();
		}

		virtual void execute(DiscordCoreAPI::BaseFunctionArguments& args) {
			try {
				DiscordCoreAPI::CreateWebHookData dataPackage;
				dataPackage.channelId = args.eventData.getChannelId();
				dataPackage.name = "TEST_WEBHOOK";

				auto newWebHook = DiscordCoreAPI::WebHooks::createWebHookAsync(dataPackage).get();

				cout << "THE NAME: " << newWebHook.name << endl;


			} catch (...) {
				rethrowException("Test::execute()");
			}
		}

		virtual ~Test() noexcept = default;
	};
}
```
