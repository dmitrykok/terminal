// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "NewProjectMenuViewModel.h"
#include <LibraryResources.h>

#include "NewProjectMenuViewModel.g.cpp"
#include "ProjectFolderTreeViewEntry.g.cpp"
#include "NewProjectMenuEntryViewModel.g.cpp"
#include "ProjectEntryViewModel.g.cpp"
#include "ProjectActionEntryViewModel.g.cpp"
#include "ProjectSeparatorEntryViewModel.g.cpp"
#include "ProjectFolderEntryViewModel.g.cpp"
#include "ProjectMatchEntryViewModel.g.cpp"
#include "ProjectRemainingEntryViewModel.g.cpp"

using namespace winrt::Windows::UI::Xaml::Navigation;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Microsoft::Terminal::Settings::Model;
using namespace winrt::Windows::UI::Xaml::Data;

namespace winrt::Microsoft::Terminal::Settings::Editor::implementation
{
	static IObservableVector<Editor::NewProjectMenuEntryViewModel> _ConvertToViewModelEntries(const IVector<Model::NewProjectMenuEntry>& settingsModelEntries, const Model::CascadiaSettings& settings)
	{
		std::vector<Editor::NewProjectMenuEntryViewModel> result{};
		if (!settingsModelEntries)
		{
			return single_threaded_observable_vector<Editor::NewProjectMenuEntryViewModel>(std::move(result));
		}

		for (const auto& entry : settingsModelEntries)
		{
			switch (entry.Type())
			{
			case NewProjectMenuEntryType::Project:
			{
				// If the Project isn't set, this is an invalid entry. Skip it.
				if (const auto& projectEntry = entry.as<Model::ProjectEntry>(); projectEntry.Project())
				{
					result.push_back(make<ProjectEntryViewModel>(projectEntry));
				}
				break;
			}
			case NewProjectMenuEntryType::ProjectAction:
			{
				if (const auto& actionEntry = entry.as<Model::ProjectActionEntry>())
				{
					result.push_back(make<ProjectActionEntryViewModel>(actionEntry, settings));
				}
				break;
			}
			case NewProjectMenuEntryType::ProjectSeparator:
			{
				if (const auto& separatorEntry = entry.as<Model::ProjectSeparatorEntry>())
				{
					result.push_back(make<ProjectSeparatorEntryViewModel>(separatorEntry));
				}
				break;
			}
			case NewProjectMenuEntryType::ProjectFolder:
			{
				if (const auto& folderEntry = entry.as<Model::ProjectFolderEntry>())
				{
					// The ctor will convert the children of the folder to view models
					result.push_back(make<ProjectFolderEntryViewModel>(folderEntry, settings));
				}
				break;
			}
			case NewProjectMenuEntryType::ProjectMatch:
			{
				if (const auto& projectMatchEntry = entry.as<Model::ProjectMatchEntry>())
				{
					result.push_back(make<ProjectMatchEntryViewModel>(projectMatchEntry));
				}
				break;
			}
			case NewProjectMenuEntryType::ProjectRemaining:
			{
				if (const auto& projectRemainingEntry = entry.as<Model::ProjectRemainingEntry>())
				{
					result.push_back(make<ProjectRemainingEntryViewModel>(projectRemainingEntry));
				}
				break;
			}
			case NewProjectMenuEntryType::Invalid:
			default:
				break;
			}
		}
		return single_threaded_observable_vector<Editor::NewProjectMenuEntryViewModel>(std::move(result));
	}

	bool NewProjectMenuViewModel::IsProjectRemainingEntryMissing() const
	{
		return _IsProjectRemainingEntryMissing(_rootEntries);
	}

	bool NewProjectMenuViewModel::_IsProjectRemainingEntryMissing(const IVector<Editor::NewProjectMenuEntryViewModel>& entries)
	{
		for (const auto& entry : entries)
		{
			switch (entry.Type())
			{
			case NewProjectMenuEntryType::ProjectRemaining:
			{
				return false;
			}
			case NewProjectMenuEntryType::ProjectFolder:
			{
				if (!_IsProjectRemainingEntryMissing(entry.as<Editor::ProjectFolderEntryViewModel>().Entries()))
				{
					return false;
				}
				break;
			}
			default:
				break;
			}
		}
		return true;
	}

	bool NewProjectMenuViewModel::IsProjectFolderView() const noexcept
	{
		return _CurrentProjectFolder != nullptr;
	}

	NewProjectMenuViewModel::NewProjectMenuViewModel(Model::CascadiaSettings settings)
	{
		UpdateSettings(settings);

		// Add a property changed handler to our own property changed event.
		// This propagates changes from the settings model to anybody listening to our
		// unique view model members.
		PropertyChanged([this](auto&&, const PropertyChangedEventArgs& args) {
			const auto viewModelProperty{ args.PropertyName() };
			if (viewModelProperty == L"AvailableProjects")
			{
				_NotifyChanges(L"SelectedProject");
			}
			else if (viewModelProperty == L"CurrentProjectFolder")
			{
				if (_CurrentProjectFolder)
				{
					CurrentFolderName(_CurrentProjectFolder.Name());
					_CurrentProjectFolder.PropertyChanged({ this, &NewProjectMenuViewModel::_FolderPropertyChanged });
				}
				_NotifyChanges(L"IsProjectFolderView", L"ProjectCurrentView");
			}
		});
	}

	void NewProjectMenuViewModel::_FolderPropertyChanged(const IInspectable& /*sender*/, const Windows::UI::Xaml::Data::PropertyChangedEventArgs& args)
	{
		const auto viewModelProperty{ args.PropertyName() };
		if (viewModelProperty == L"Name")
		{
			// FolderTree needs to be updated when a folder is renamed
			_folderTreeCache = nullptr;
		}
	}

	hstring NewProjectMenuViewModel::CurrentFolderName() const
	{
		if (!_CurrentProjectFolder)
		{
			return {};
		}
		return _CurrentProjectFolder.Name();
	}

	void NewProjectMenuViewModel::CurrentFolderName(const hstring& value)
	{
		if (_CurrentProjectFolder && _CurrentProjectFolder.Name() != value)
		{
			_CurrentProjectFolder.Name(value);
			_NotifyChanges(L"CurrentFolderName");
		}
	}

	bool NewProjectMenuViewModel::CurrentFolderInlining() const
	{
		if (!_CurrentProjectFolder)
		{
			return {};
		}
		return _CurrentProjectFolder.Inlining();
	}

	void NewProjectMenuViewModel::CurrentFolderInlining(bool value)
	{
		if (_CurrentProjectFolder && _CurrentProjectFolder.Inlining() != value)
		{
			_CurrentProjectFolder.Inlining(value);
			_NotifyChanges(L"CurrentFolderInlining");
		}
	}

	bool NewProjectMenuViewModel::CurrentFolderAllowEmpty() const
	{
		if (!_CurrentProjectFolder)
		{
			return {};
		}
		return _CurrentProjectFolder.AllowEmpty();
	}

	void NewProjectMenuViewModel::CurrentFolderAllowEmpty(bool value)
	{
		if (_CurrentProjectFolder && _CurrentProjectFolder.AllowEmpty() != value)
		{
			_CurrentProjectFolder.AllowEmpty(value);
			_NotifyChanges(L"CurrentFolderAllowEmpty");
		}
	}

	Windows::Foundation::Collections::IObservableVector<Editor::NewProjectMenuEntryViewModel> NewProjectMenuViewModel::ProjectCurrentView() const
	{
		if (!_CurrentProjectFolder)
		{
			return _rootEntries;
		}
		return _CurrentProjectFolder.Entries();
	}

	static bool _FindFolderPathByName(const IVector<Editor::NewProjectMenuEntryViewModel>& entries, const hstring& name, std::vector<Editor::ProjectFolderEntryViewModel>& result)
	{
		for (const auto& entry : entries)
		{
			if (const auto& folderVM = entry.try_as<Editor::ProjectFolderEntryViewModel>())
			{
				result.push_back(folderVM);
				if (folderVM.Name() == name)
				{
					// Found the folder
					return true;
				}
				else if (_FindFolderPathByName(folderVM.Entries(), name, result))
				{
					// Found the folder in the children of this folder
					return true;
				}
				else
				{
					// This folder and its descendants are not the folder we're looking for
					result.pop_back();
				}
			}
		}
		return false;
	}

	IVector<Editor::ProjectFolderEntryViewModel> NewProjectMenuViewModel::FindFolderPathByName(const hstring& name)
	{
		std::vector<Editor::ProjectFolderEntryViewModel> entries;
		_FindFolderPathByName(_rootEntries, name, entries);
		return single_threaded_vector<Editor::ProjectFolderEntryViewModel>(std::move(entries));
	}

	void NewProjectMenuViewModel::UpdateSettings(const Model::CascadiaSettings& settings)
	{
		_Settings = settings;
		_NotifyChanges(L"AvailableProjects");

		SelectedProject(AvailableProjects().GetAt(0));

		// TODO: remove pass nullptr
		//_rootEntries = _ConvertToViewModelEntries(nullptr, _Settings);
		//_rootEntriesChangedRevoker = _rootEntries.VectorChanged(winrt::auto_revoke, [this](auto&&, const IVectorChangedEventArgs& args) {
		//	switch (args.CollectionChange())
		//	{
		//	case CollectionChange::Reset:
		//	{
		//		// fully replace settings model with view model structure
		//		std::vector<Model::NewProjectMenuEntry> modelEntries;
		//		for (const auto& entry : _rootEntries)
		//		{
		//			modelEntries.push_back(NewProjectMenuEntryViewModel::GetModel(entry));
		//		}
		//		_Settings.GlobalSettings().NewProjectMenu(single_threaded_vector<Model::NewProjectMenuEntry>(std::move(modelEntries)));
		//		return;
		//	}
		//	case CollectionChange::ItemInserted:
		//	{
		//		const auto& insertedEntryVM = _rootEntries.GetAt(args.Index());
		//		const auto& insertedEntry = NewProjectMenuEntryViewModel::GetModel(insertedEntryVM);
		//		_Settings.GlobalSettings().NewProjectMenu().InsertAt(args.Index(), insertedEntry);
		//		return;
		//	}
		//	case CollectionChange::ItemRemoved:
		//	{
		//		_Settings.GlobalSettings().NewProjectMenu().RemoveAt(args.Index());
		//		return;
		//	}
		//	case CollectionChange::ItemChanged:
		//	{
		//		const auto& modifiedEntry = _rootEntries.GetAt(args.Index());
		//		_Settings.GlobalSettings().NewProjectMenu().SetAt(args.Index(), NewProjectMenuEntryViewModel::GetModel(modifiedEntry));
		//		return;
		//	}
		//	}
		//});
	}

	void NewProjectMenuViewModel::RequestReorderEntry(const Editor::NewProjectMenuEntryViewModel& vm, bool goingUp)
	{
		uint32_t idx;
		if (ProjectCurrentView().IndexOf(vm, idx))
		{
			if (goingUp && idx > 0)
			{
				ProjectCurrentView().RemoveAt(idx);
				ProjectCurrentView().InsertAt(idx - 1, vm);
			}
			else if (!goingUp && idx < ProjectCurrentView().Size() - 1)
			{
				ProjectCurrentView().RemoveAt(idx);
				ProjectCurrentView().InsertAt(idx + 1, vm);
			}
		}
	}

	void NewProjectMenuViewModel::RequestDeleteEntry(const Editor::NewProjectMenuEntryViewModel& vm)
	{
		uint32_t idx;
		if (ProjectCurrentView().IndexOf(vm, idx))
		{
			ProjectCurrentView().RemoveAt(idx);

			if (vm.try_as<Editor::ProjectFolderEntryViewModel>())
			{
				_folderTreeCache = nullptr;
			}
		}
	}

	void NewProjectMenuViewModel::RequestMoveEntriesToFolder(const Windows::Foundation::Collections::IVector<Editor::NewProjectMenuEntryViewModel>& entries, const Editor::ProjectFolderEntryViewModel& destinationFolder)
	{
		auto destination{ destinationFolder == nullptr ? _rootEntries : destinationFolder.Entries() };
		for (auto&& e : entries)
		{
			// Don't move the folder into itself (just skip over it)
			if (e == destinationFolder)
			{
				continue;
			}

			// Remove entry from the current layer,
			// and add it to the destination folder
			RequestDeleteEntry(e);
			destination.Append(e);
		}
	}

	Editor::NewProjectMenuEntryViewModel NewProjectMenuViewModel::RequestAddSelectedProjectEntry()
	{
		if (_SelectedProject)
		{
			Model::ProjectEntry projectEntry;
			projectEntry.Project(_SelectedProject);

			const auto& entryVM = make<ProjectEntryViewModel>(projectEntry);
			ProjectCurrentView().Append(entryVM);
			return entryVM;
		}
		return nullptr;
	}

	Editor::NewProjectMenuEntryViewModel NewProjectMenuViewModel::RequestAddProjectSeparatorEntry()
	{
		Model::ProjectSeparatorEntry separatorEntry;
		const auto& entryVM = make<ProjectSeparatorEntryViewModel>(separatorEntry);
		ProjectCurrentView().Append(entryVM);
		return entryVM;
	}

	Editor::NewProjectMenuEntryViewModel NewProjectMenuViewModel::RequestAddProjectFolderEntry()
	{
		Model::ProjectFolderEntry folderEntry;
		folderEntry.Name(_AddProjectFolderName);

		const auto& entryVM = make<ProjectFolderEntryViewModel>(folderEntry, _Settings);
		ProjectCurrentView().Append(entryVM);

		// Reset state after adding the entry
		AddProjectFolderName({});
		_folderTreeCache = nullptr;
		return entryVM;
	}

	Editor::NewProjectMenuEntryViewModel NewProjectMenuViewModel::RequestAddProjectMatcherEntry()
	{
		Model::ProjectMatchEntry matchProjectsEntry;
		matchProjectsEntry.Name(_ProjectMatcherName);
		matchProjectsEntry.Source(_ProjectMatcherSource);
		matchProjectsEntry.Commandline(_ProjectMatcherCommandline);

		const auto& entryVM = make<ProjectMatchEntryViewModel>(matchProjectsEntry);
		ProjectCurrentView().Append(entryVM);

		// Clear the fields after adding the entry
		ProjectMatcherName({});
		ProjectMatcherSource({});
		ProjectMatcherCommandline({});

		return entryVM;
	}

	Editor::NewProjectMenuEntryViewModel NewProjectMenuViewModel::RequestAddProjectRemainingEntry()
	{
		Model::ProjectRemainingEntry remainingProjectsEntry;
		const auto& entryVM = make<ProjectRemainingEntryViewModel>(remainingProjectsEntry);
		ProjectCurrentView().Append(entryVM);

		_NotifyChanges(L"IsProjectRemainingEntryMissing");

		return entryVM;
	}

	void NewProjectMenuViewModel::GenerateFolderTree()
	{
		if (!_folderTreeCache)
		{
			// Add the root folder
			auto root = winrt::make<ProjectFolderTreeViewEntry>(nullptr);

			for (const auto&& entry : _rootEntries)
			{
				if (entry.Type() == NewProjectMenuEntryType::ProjectFolder)
				{
					root.Children().Append(winrt::make<ProjectFolderTreeViewEntry>(entry.as<Editor::ProjectFolderEntryViewModel>()));
				}
			}

			std::vector<Editor::ProjectFolderTreeViewEntry> folderTreeCache;
			folderTreeCache.emplace_back(std::move(root));
			_folderTreeCache = single_threaded_observable_vector<Editor::ProjectFolderTreeViewEntry>(std::move(folderTreeCache));

			_NotifyChanges(L"ProjectFolderTree");
		}
	}

	Collections::IObservableVector<Editor::ProjectFolderTreeViewEntry> NewProjectMenuViewModel::ProjectFolderTree() const
	{
		// We could do this...
		//   if (!_folderTreeCache){ GenerateFolderTree(); }
		// But ProjectFolderTree() gets called when we open the page.
		// Instead, we generate the tree as needed using GenerateFolderTree()
		//  which caches the tree.
		return _folderTreeCache;
	}

	// This recursively constructs the ProjectFolderTree
	ProjectFolderTreeViewEntry::ProjectFolderTreeViewEntry(Editor::ProjectFolderEntryViewModel folderEntry) :
		_folderEntry{ folderEntry },
		_Children{ single_threaded_observable_vector<Editor::ProjectFolderTreeViewEntry>() }
	{
		if (!_folderEntry)
		{
			return;
		}

		for (const auto&& entry : _folderEntry.Entries())
		{
			if (entry.Type() == NewProjectMenuEntryType::ProjectFolder)
			{
				_Children.Append(winrt::make<ProjectFolderTreeViewEntry>(entry.as<Editor::ProjectFolderEntryViewModel>()));
			}
		}
	}

	hstring ProjectFolderTreeViewEntry::Name() const
	{
		if (!_folderEntry)
		{
			return RS_(L"NewProjectMenu_RootFolderName");
		}
		return _folderEntry.Name();
	}

	hstring ProjectFolderTreeViewEntry::Icon() const
	{
		if (!_folderEntry)
		{
			return {};
		}
		return _folderEntry.Icon();
	}

	NewProjectMenuEntryViewModel::NewProjectMenuEntryViewModel(const NewProjectMenuEntryType type) noexcept :
		_Type{ type }
	{
	}

	Model::NewProjectMenuEntry NewProjectMenuEntryViewModel::GetModel(const Editor::NewProjectMenuEntryViewModel& viewModel)
	{
		switch (viewModel.Type())
		{
		case NewProjectMenuEntryType::Project:
		{
			const auto& projVM = viewModel.as<Editor::ProjectEntryViewModel>();
			return get_self<ProjectEntryViewModel>(projVM)->ProjectEntry();
		}
		case NewProjectMenuEntryType::ProjectAction:
		{
			const auto& projVM = viewModel.as<Editor::ProjectActionEntryViewModel>();
			return get_self<ProjectActionEntryViewModel>(projVM)->ProjectActionEntry();
		}
		case NewProjectMenuEntryType::ProjectSeparator:
		{
			const auto& projVM = viewModel.as<Editor::ProjectSeparatorEntryViewModel>();
			return get_self<ProjectSeparatorEntryViewModel>(projVM)->ProjectSeparatorEntry();
		}
		case NewProjectMenuEntryType::ProjectFolder:
		{
			const auto& projVM = viewModel.as<Editor::ProjectFolderEntryViewModel>();
			return get_self<ProjectFolderEntryViewModel>(projVM)->ProjectFolderEntry();
		}
		case NewProjectMenuEntryType::ProjectMatch:
		{
			const auto& projVM = viewModel.as<Editor::ProjectMatchEntryViewModel>();
			return get_self<ProjectMatchEntryViewModel>(projVM)->ProjectMatchEntry();
		}
		case NewProjectMenuEntryType::ProjectRemaining:
		{
			const auto& projVM = viewModel.as<Editor::ProjectRemainingEntryViewModel>();
			return get_self<ProjectRemainingEntryViewModel>(projVM)->ProjectRemainingEntry();
		}
		case NewProjectMenuEntryType::Invalid:
		default:
			return nullptr;
		}
	}

	ProjectEntryViewModel::ProjectEntryViewModel(Model::ProjectEntry projectEntry) :
		ProjectEntryViewModelT<ProjectEntryViewModel, NewProjectMenuEntryViewModel>(Model::NewProjectMenuEntryType::Project),
		_ProjectEntry{ projectEntry }
	{
	}

	ProjectActionEntryViewModel::ProjectActionEntryViewModel(Model::ProjectActionEntry actionEntry, Model::CascadiaSettings settings) :
		ProjectActionEntryViewModelT<ProjectActionEntryViewModel, NewProjectMenuEntryViewModel>(Model::NewProjectMenuEntryType::ProjectAction),
		_ProjectActionEntry{ actionEntry },
		_Settings{ settings }
	{
	}

	hstring ProjectActionEntryViewModel::DisplayText() const
	{
		assert(_Settings);

		const auto actionID = _ProjectActionEntry.ActionId();
		if (const auto& action = _Settings.ActionMap().GetActionByID(actionID))
		{
			return action.Name();
		}
		return hstring{ fmt::format(L"{}: {}", RS_(L"NewProjectMenu_ActionNotFound"), actionID) };
	}

	hstring ProjectActionEntryViewModel::Icon() const
	{
		assert(_Settings);

		const auto actionID = _ProjectActionEntry.ActionId();
		if (const auto& action = _Settings.ActionMap().GetActionByID(actionID))
		{
			return action.IconPath();
		}
		return {};
	}

	ProjectSeparatorEntryViewModel::ProjectSeparatorEntryViewModel(Model::ProjectSeparatorEntry separatorEntry) :
		ProjectSeparatorEntryViewModelT<ProjectSeparatorEntryViewModel, NewProjectMenuEntryViewModel>(Model::NewProjectMenuEntryType::ProjectSeparator),
		_ProjectSeparatorEntry{ separatorEntry }
	{
	}

	ProjectFolderEntryViewModel::ProjectFolderEntryViewModel(Model::ProjectFolderEntry folderEntry) :
		ProjectFolderEntryViewModel(folderEntry, nullptr) {}

	ProjectFolderEntryViewModel::ProjectFolderEntryViewModel(Model::ProjectFolderEntry folderEntry, Model::CascadiaSettings settings) :
		ProjectFolderEntryViewModelT<ProjectFolderEntryViewModel, NewProjectMenuEntryViewModel>(Model::NewProjectMenuEntryType::ProjectFolder),
		_ProjectFolderEntry{ folderEntry },
		_Settings{ settings }
	{
		_Entries = _ConvertToViewModelEntries(_ProjectFolderEntry.RawEntries(), _Settings);

		_entriesChangedRevoker = _Entries.VectorChanged(winrt::auto_revoke, [this](auto&&, const IVectorChangedEventArgs& args) {
			switch (args.CollectionChange())
			{
			case CollectionChange::Reset:
			{
				// fully replace settings model with _Entries
				std::vector<Model::NewProjectMenuEntry> modelEntries;
				for (const auto& entry : _Entries)
				{
					modelEntries.push_back(NewProjectMenuEntryViewModel::GetModel(entry));
				}
				_ProjectFolderEntry.RawEntries(single_threaded_vector<Model::NewProjectMenuEntry>(std::move(modelEntries)));
				return;
			}
			case CollectionChange::ItemInserted:
			{
				const auto& insertedEntryVM = _Entries.GetAt(args.Index());
				const auto& insertedEntry = NewProjectMenuEntryViewModel::GetModel(insertedEntryVM);
				if (!_ProjectFolderEntry.RawEntries())
				{
					_ProjectFolderEntry.RawEntries(single_threaded_vector<Model::NewProjectMenuEntry>());
				}
				_ProjectFolderEntry.RawEntries().InsertAt(args.Index(), insertedEntry);
				return;
			}
			case CollectionChange::ItemRemoved:
			{
				_ProjectFolderEntry.RawEntries().RemoveAt(args.Index());
				return;
			}
			case CollectionChange::ItemChanged:
			{
				const auto& modifiedEntry = _Entries.GetAt(args.Index());
				_ProjectFolderEntry.RawEntries().SetAt(args.Index(), NewProjectMenuEntryViewModel::GetModel(modifiedEntry));
				return;
			}
			}
		});
	}

	bool ProjectFolderEntryViewModel::Inlining() const
	{
		return _ProjectFolderEntry.Inlining() == ProjectFolderEntryInlining::Auto;
	}

	void ProjectFolderEntryViewModel::Inlining(bool value)
	{
		const auto valueAsEnum = value ? ProjectFolderEntryInlining::Auto : ProjectFolderEntryInlining::Never;
		if (_ProjectFolderEntry.Inlining() != valueAsEnum)
		{
			_ProjectFolderEntry.Inlining(valueAsEnum);
			_NotifyChanges(L"Inlining");
		}
	};

	ProjectMatchEntryViewModel::ProjectMatchEntryViewModel(Model::ProjectMatchEntry matchProjectsEntry) :
		ProjectMatchEntryViewModelT<ProjectMatchEntryViewModel, NewProjectMenuEntryViewModel>(Model::NewProjectMenuEntryType::ProjectMatch),
		_ProjectMatchEntry{ matchProjectsEntry }
	{
	}

	hstring ProjectMatchEntryViewModel::DisplayText() const
	{
		std::wstring displayText;
		if (const auto projectName = _ProjectMatchEntry.Name(); !projectName.empty())
		{
			fmt::format_to(std::back_inserter(displayText), FMT_COMPILE(L"project: {}, "), projectName);
		}
		if (const auto commandline = _ProjectMatchEntry.Commandline(); !commandline.empty())
		{
			fmt::format_to(std::back_inserter(displayText), FMT_COMPILE(L"commandline: {}, "), commandline);
		}
		if (const auto source = _ProjectMatchEntry.Source(); !source.empty())
		{
			fmt::format_to(std::back_inserter(displayText), FMT_COMPILE(L"source: {}, "), source);
		}

		// Chop off the last ", "
		displayText.resize(displayText.size() - 2);
		return winrt::hstring{ displayText };
	}

	ProjectRemainingEntryViewModel::ProjectRemainingEntryViewModel(Model::ProjectRemainingEntry remainingProjectsEntry) :
		ProjectRemainingEntryViewModelT<ProjectRemainingEntryViewModel, NewProjectMenuEntryViewModel>(Model::NewProjectMenuEntryType::ProjectRemaining),
		_ProjectRemainingEntry{ remainingProjectsEntry }
	{
	}
}
