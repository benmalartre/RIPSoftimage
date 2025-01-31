# we assume that there is a pattern in your re-organisation
# for exemple you renamed the project or moved it to a new disk

to_replace = "the_original_invalid_path_part"
replace_by = "the_new_valid_path_part"


def FindAudioClipsUnderModel(in_model):
	if not in_model.HasMixer():
		return []
	mixer = in_model.Mixer
	return [clip for clip in mixer.Clips if clip.Type == "mixeraudioclip"]


audio_clips = FindAudioClipsUnderModel(Application.ActiveSceneRoot)
for audio_clip in audio_clips:
	source = audio_clip.Source
	old_value = source.FileName.Value
	new_value = old_value.replace(to_replace, replace_by)
	
	# source.FileName.Value = new_value 
	# object model not working here
	# use command instead
	Application.SetValue(source.FileName, new_value, "")

Application.SaveScene()
