# RFID Tag Authentication Server
# Simple server app based on the Sinatra DSL.

require 'sinatra'
require 'json'

set :port, 55555
set :environment, :production

helpers do
  # Get authorized tag IDs for parsing
  def get_tags
    tags = JSON.parse(IO.read('cards.json'))
  end
end

# Next three paths are lazy file serving
get "/watch" do
  IO.read "watcher.html"
end

get "/style.css" do
  send_file "style.css"
end

get "/requests.json/*" do
  IO.read "requests.json"
end
# End lazy serving

# Processes requests from Arduino client
get '/id/:id' do
  found = false

  # Sequential search for authorized tag
  get_tags.each { |tag|
    if tag == params[:id]
      found = true
    end
  }

  # Read old request log into variable
  requests = JSON.parse(IO.read('requests.json'))
  # Create new log entry
  newRequest = [Time.now.utc.getlocal, params[:id], found]
  # Add entry to log
  requests.push newRequest
  # Stringify it
  data = JSON.generate requests
  # Save into file
  File.open("requests.json", 'w') {|f| f.write(data) }

  # Return HTTP status code based on authorization results
  found ? 200 : 404
end

