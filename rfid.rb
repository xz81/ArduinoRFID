# Authentication Server
# 
#
#

require 'sinatra'
require 'json'

set :port, 55555
set :environment, :production

helpers do
  def get_tags
    tags = JSON.parse(IO.read('cards.json'))
  end
end

get "/watch" do
  IO.read "watcher.html"
end

get "/style.css" do
  send_file "style.css"
end

get "/requests.json/*" do
  IO.read "requests.json"
end

get '/id/:id' do
  found = false

  get_tags.each { |tag|
    if tag == params[:id]
      found = true
    end
  }

  requests = JSON.parse(IO.read('requests.json'))
  newRequest = [Time.now.utc.getlocal, params[:id], found]
  requests.push newRequest
  data = JSON.generate requests
  File.open("requests.json", 'w') {|f| f.write(data) }

  found ? 200 : 404

end

