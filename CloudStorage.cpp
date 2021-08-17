#include "apps/eclipseworkspace/facial/CloudStorage.hpp"

#include <iostream>

namespace isaac {

void CloudStorage::start() {
	//google::cloud::storage::ClientOptions options(google::cloud::storage::oauth2::CreateAuthorizedUserCredentialsFromJsonFilePath("apps/eclipseworkspace/facial/"+get_file_name()).value());
	//client = google::cloud::storage::Client(options);
	client = google::cloud::storage::Client::CreateDefaultClient();
	if(!client) LOG_ERROR("Could not create google cloud client");
	tickOnMessage(rx_embeddings());
}

void CloudStorage::tick() {
	TensorConstView2f const_embeddings;
	FromProto(rx_embeddings().getProto(), rx_embeddings().buffers(), const_embeddings);

	auto writer = client->WriteObject(get_bucket_name(), "embeddings");
	writer << const_embeddings(0);

	writer.Close();

	/*double sum=0;
	for(int i = 0;i<512;i++){
		sum+=pow((const_embeddings(0,i)-past_embeddings(0,i)),2.0);
	}
	sum = sqrt(sum);

	if(no_past){
		no_past = 0;
		Copy(const_embeddings,past_embeddings);
		return;
	}
	show("embedding distance",sum);
	if(abs(sum)>1.0){
		std::cout << "updated past with distance: " << sum <<  '\n';
		Copy(const_embeddings,past_embeddings);
	}
	*/
}
}
