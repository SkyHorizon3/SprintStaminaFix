#pragma once

namespace SprintStaminaFix
{
	/*
	class SprintStaminaHook
	{
		struct TrampolineCall : Xbyak::CodeGenerator
		{
			TrampolineCall(std::uintptr_t retn, std::uintptr_t func)
			{
				Xbyak::Label funcLabel;
				Xbyak::Label originFuncLabel;
				Xbyak::Label retnLabel;

				call(ptr[rip + originFuncLabel]); //call thunk

				mov(rdx, rdi); // move actor
				call(ptr[rip + funcLabel]); //call thunk

				jmp(ptr[rip + retnLabel]); //jump back to original code

				L(funcLabel);
				dq(func);

				L(originFuncLabel);
				dq(REL::VariantID(25900, 26482, 0x0).address());

				L(retnLabel);
				dq(retn);
			}
		};


		static float ResetStaminaCost(float originResult, RE::Actor* a_me)
		{
			const auto IsGVTrue = [a_me](const std::string& varName) -> bool {
				bool result = false;
				return a_me->GetGraphVariableBool(varName, result) && result;
				};

			if (a_me && (IsGVTrue("MCO_InSprintAttack") || IsGVTrue("MCO_InSprintPowerAttack")))
				return 0.f;


			return originResult;
		}

	public:
		static void InstallHook()
		{
			SKSE::AllocTrampoline(1 << 6);

			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36994, 38022), REL::VariantOffset(0xC9,0xC9,0x12D) };
			auto funcAddr = target.address();

			REL::safe_fill(funcAddr, REL::NOP, 0x5);

			auto trampolineJmp = TrampolineCall(funcAddr + 0x5, stl::unrestricted_cast<std::uintptr_t>(ResetStaminaCost));

			auto& trampoline = SKSE::GetTrampoline();
			auto result = trampoline.allocate(trampolineJmp);
			trampoline.write_branch<5>(funcAddr, (std::uintptr_t)result);

			SKSE::log::info("{} Done!", __FUNCTION__);
		}
	};
	*/

	// More efficient solution:

	static RE::Actor* g_me = nullptr;
	struct GetEquippedWeightHook
	{
		static float thunk(RE::Actor* a_me)
		{
			g_me = a_me;
			return func(a_me);
		};
		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct CalcStaminaCostHook
	{
		static float thunk(float equippedWeight, float secondsSinceLastFrame_WorldTime)
		{
			auto originalResult = func(equippedWeight, secondsSinceLastFrame_WorldTime);

			const auto IsGVTrue = [](const std::string& varName) -> bool {
				bool result = false;
				return g_me->GetGraphVariableBool(varName, result) && result;
				};

			if (g_me && (IsGVTrue("MCO_InSprintAttack") || IsGVTrue("MCO_InSprintPowerAttack")))
				return 0.f;

			return originalResult;
		};
		static inline REL::Relocation<decltype(thunk)> func;
	};

	void InstallHooks()
	{
		SKSE::AllocTrampoline(28);

		constexpr auto funcAddress = RELOCATION_ID(36994, 38022);
		auto& trampoline = SKSE::GetTrampoline();

		REL::Relocation<std::uintptr_t> loc1{ funcAddress, REL::Relocate(0xC1, 0xC1, 0x125) };
		GetEquippedWeightHook::func = trampoline.write_call<5>(loc1.address(), GetEquippedWeightHook::thunk);

		REL::Relocation<std::uintptr_t> loc2{ funcAddress, REL::Relocate(0xC9, 0xC9, 0x12D) };
		CalcStaminaCostHook::func = trampoline.write_call<5>(loc2.address(), CalcStaminaCostHook::thunk);

		SKSE::log::info("Installed Hooks!");
	}

}